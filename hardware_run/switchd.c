#define _GNU_SOURCE
#include <dlfcn.h> 
#include <pipe_mgr/pipe_mgr_intf.h>
#include <unistd.h>
#include <sys/time.h> 
#include <sys/ioctl.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>

#include <arpa/inet.h>

#include <tofino/pdfixed/pd_conn_mgr.h>
#include <tofino/pdfixed/pd_tm.h>
#include <traffic_mgr/traffic_mgr_types.h>
#include <traffic_mgr/traffic_mgr_q_intf.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <bf_switchd/bf_switchd.h>
#ifdef __cplusplus
}
#endif

// Typically 4 lane and better not use masked ports
#define PKTGEN_SRC_PORT 68
#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PD_DEV_PIPE_ALL 0xffff

#define PKTGEN_APP_1 0x1
#define PKTGEN_APP_1_OFFSET 0

// Has to be >=54B
#define MIN_PKTGEN_SIZE 54

#define DEV_ID 0

p4_pd_sess_hdl_t sess_hdl;

// 6B auto-padding by pktgen hdr, pktgen without dst mac for optimization
typedef struct __attribute__((__packed__)) p4sync_t {
    uint8_t srcAddr[6];
    uint16_t type;
    char ipv4[20];
} p4sync;

p4sync p4sync_pkt;
uint8_t *upkt;
size_t sz = sizeof(p4sync);

void pkt_init(uint16_t pkt_type) {
    uint8_t srcAddr[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    memcpy(p4sync_pkt.srcAddr, srcAddr, 6);
    p4sync_pkt.type = htons(pkt_type); 
    p4sync_pkt.ipv4[9] = 0x06;  //tcp type
    upkt = (uint8_t *) malloc(sz);
    memcpy(upkt, &p4sync_pkt, sz);
}

void pkt_init_ip(uint8_t pkt_type) {
    uint8_t srcAddr[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    memcpy(p4sync_pkt.srcAddr, srcAddr, 6);
    p4sync_pkt.type = htons(0);
    // 0-3b Version
    // 4-7b IHL (number of 32b words), min 5
    p4sync_pkt.ipv4[0] = 0;  // O.w., bogus ipv4 version, or invalid IHL
    p4sync_pkt.ipv4[9] = 0;  //type
    upkt = (uint8_t *) malloc(sz);
    memcpy(upkt, &p4sync_pkt, sz);
}

void pktgen_init(int pipe_id, int app_id, int pkt_offset, int timer_ns, int batch_size) {
    
    int buffer_len = (sz < MIN_PKTGEN_SIZE)? MIN_PKTGEN_SIZE:sz;

    p4_pd_dev_target_t p4_pd_device;
    p4_pd_device.device_id = 0;
    p4_pd_device.dev_pipe_id = pipe_id;

    p4_pd_status_t pd_status;

    // Or full pkt but buffer_len-6, upkt+6
    pd_status = p4_pd_pktgen_write_pkt_buffer(sess_hdl, p4_pd_device, pkt_offset, buffer_len, upkt);
    
    if (pd_status != 0) {
        printf("Pktgen: Writing Packet buffer failed!\n");
        return;
    }
    p4_pd_complete_operations(sess_hdl);
    
    pd_status = p4_pd_pktgen_enable(sess_hdl, 0, 68 + 128 * pipe_id);

    if (pd_status != 0) {
        printf("Failed to enable pktgen status = %d!!\n", pd_status);
        return;
    }

    struct p4_pd_pktgen_app_cfg prob_app_cfg;
        
    prob_app_cfg.trigger_type = PD_PKTGEN_TRIGGER_TIMER_PERIODIC;
    // 0 indexed
    prob_app_cfg.batch_count = 0;
    prob_app_cfg.packets_per_batch = batch_size;
    prob_app_cfg.pattern_value = 0;
    prob_app_cfg.pattern_mask = 0;
    prob_app_cfg.timer_nanosec = timer_ns;
    prob_app_cfg.ibg = 0;
    prob_app_cfg.ibg_jitter = 0;
    prob_app_cfg.ipg = 0;
    prob_app_cfg.ipg_jitter = 0;
    prob_app_cfg.source_port = PKTGEN_SRC_PORT;
    prob_app_cfg.increment_source_port = 0;

    prob_app_cfg.pkt_buffer_offset = pkt_offset;
    prob_app_cfg.length = buffer_len;

    pd_status = p4_pd_pktgen_cfg_app(sess_hdl,
            p4_pd_device,
            app_id,
            prob_app_cfg);

    if (pd_status != 0) {
        printf("pktgen app configuration failed\n");
        return;
    }

    pd_status = p4_pd_pktgen_app_enable(sess_hdl, p4_pd_device, app_id);
    if (pd_status != 0) {
        printf("Pktgen : App enable Failed!\n");
        return;
    }
   sleep(100);
   pd_status = p4_pd_pktgen_app_disable(sess_hdl, p4_pd_device, app_id);
   if (pd_status != 0) {
       printf("Pktgen : App disable Failed!\n");
       return;
   }
}

char *read_key(FILE *file, char const *targetkey) { 
    char key[128];
    char val[128];
    while (fscanf(file, "%127[^=]=%127[^\n]%*c", key, val) == 2) {
        if (0 == strcmp(key, targetkey)) {
            return strdup(val);
    }
    }
    rewind(file);  // O.w. the args needs to be sorted
    return NULL;
}

int main(int argc, char **argv) {

  bf_switchd_context_t *switchd_ctx;
  if ((switchd_ctx = (bf_switchd_context_t *) calloc(1, sizeof(bf_switchd_context_t))) == NULL) {
    printf("Cannot Allocate switchd context\n");
    exit(1);
  }
  switchd_ctx->install_dir = argv[1];
  switchd_ctx->conf_file = argv[2];
  switchd_ctx->running_in_background = false;
  switchd_ctx->skip_port_add = false; 
  switchd_ctx->skip_p4 = false;
  switchd_ctx->dev_sts_thread = true;
  switchd_ctx->kernel_pkt = false;
  switchd_ctx->dev_sts_port = 7777;
  /////////////////
  printf("$$$$$$$$$$$$$$$\n");
  const char *ini_fn = argv[3];
  printf("ini file path: %s\n", ini_fn);
  FILE *f = fopen(ini_fn, "r");
  if (f == NULL) {
      printf("%s: %s not found\n", __FUNCTION__, ini_fn);
      return 1;
  }
  char* ether_pktgen_c = NULL;
  uint16_t ether_pktgen = 0x4321;
  char* ip_pktgen_c = NULL;
  uint8_t ip_pktgen = 0x90;
  if (ether_pktgen_c = read_key(f, "ether_pktgen")) {
      ether_pktgen = (uint16_t)strtol(ether_pktgen_c, NULL, 0);
      printf("ether_pktgen=%s, hex value %x\n", ether_pktgen_c, ether_pktgen);
      pkt_init(ether_pktgen);
  } else if (ip_pktgen_c = read_key(f, "ip_pktgen")) {
      ip_pktgen = (uint8_t)strtol(ip_pktgen_c, NULL, 0);
      printf("ip_pktgen=%s, hex value %x\n", ip_pktgen_c, ip_pktgen);
      pkt_init_ip(ip_pktgen);
  } else {
      printf("Unspecified pktgen type!\n");
      return 1;
  }

  char* pktgen_period_c = NULL;
  int pktgen_period = 8;
  if (pktgen_period_c = read_key(f, "pktgen_period")) {
      pktgen_period = atoi(pktgen_period_c);
      printf("pktgen_period=%s, int %d\n", pktgen_period_c, pktgen_period);
  }
  char* pktgen_pipeid_c = NULL;
  int pktgen_pipeid = 0;
  if (pktgen_pipeid_c = read_key(f, "pktgen_pipeid")) {
      pktgen_pipeid = atoi(pktgen_pipeid_c);
      printf("pktgen_pipeid=%s, int %d\n", pktgen_pipeid_c, pktgen_pipeid);
  }
  char* kernel_pkt_c = NULL;
  if (kernel_pkt_c = read_key(f, "kernel_pkt")) {
      if(strcmp(kernel_pkt_c, "true")==0) {
          printf("pktgen_pkt=%s, set true\n", kernel_pkt_c);
          switchd_ctx->kernel_pkt = true; 
      } else {
          printf("pktgen_pkt=%s, set false\n", kernel_pkt_c);
          switchd_ctx->kernel_pkt = false;
      }
  } 
  
  char* gen_pktgen_c = NULL;
  bool gen_pktgen = true;
  if (gen_pktgen_c = read_key(f, "gen_pktgen")) {
      if(strcmp(gen_pktgen_c, "false")==0) {
          gen_pktgen = false;
      }
  }
  printf("gen_pktgen: %s\n", gen_pktgen ? "true" : "false");

  printf("$$$$$$$$$$$$$$\n");
  ///////////////////////
  bf_switchd_lib_init(switchd_ctx);

  p4_pd_status_t status;
  status = p4_pd_client_init(&sess_hdl);
    if (status == 0) {
        printf("Successfully performed client initialization.\n");
    } else {
        printf("Failed in Client init\n");
    }

  // bf_ts_global_ts_inc_value_set()

  printf("Wait 30s for setting up ports......\n");
  sleep(30);
  
  if(gen_pktgen) {
      printf("Configure pktgen....\n");
      pktgen_init(pktgen_pipeid, PKTGEN_APP_1, PKTGEN_APP_1_OFFSET, pktgen_period, 0);
  }

  pthread_join(switchd_ctx->tmr_t_id, NULL);                                                                                          
  pthread_join(switchd_ctx->dma_t_id, NULL);
  pthread_join(switchd_ctx->int_t_id, NULL);
  pthread_join(switchd_ctx->pkt_t_id, NULL);
  pthread_join(switchd_ctx->port_fsm_t_id, NULL);
  pthread_join(switchd_ctx->drusim_t_id, NULL);
  pthread_join(switchd_ctx->accton_diag_t_id, NULL);
  for (int agent_idx = 0; agent_idx < BF_SWITCHD_MAX_AGENTS; agent_idx++) {
   if (switchd_ctx->agent_t_id[agent_idx] != 0) {
     pthread_join(switchd_ctx->agent_t_id[agent_idx], NULL);
   }
  }
  if (switchd_ctx) free(switchd_ctx);

  return 0;
}
