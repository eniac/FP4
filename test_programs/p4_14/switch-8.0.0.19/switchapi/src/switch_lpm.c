/*******************************************************************************
 * BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
 *
 * Copyright (c) 2015-2016 Barefoot Networks, Inc.

 * All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains the property of
 * Barefoot Networks, Inc. and its suppliers, if any. The intellectual and
 * technical concepts contained herein are proprietary to Barefoot Networks,
 * Inc.
 * and its suppliers and may be covered by U.S. and Foreign Patents, patents in
 * process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material is
 * strictly forbidden unless prior written permission is obtained from
 * Barefoot Networks, Inc.
 *
 * No warranty, explicit or implicit is provided, unless granted under a
 * written agreement with Barefoot Networks, Inc.
 *
 * $Id: $
 *
 ******************************************************************************/

#include "switchapi/switch_utils.h"
#include <bfutils/Judy.h>

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"
#include "switch_lpm_int.h"

static inline switch_status_t switch_trie_node_allocate(
    switch_device_t device, switch_trie_node_t **node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  *node = SWITCH_MALLOC(device, sizeof(switch_trie_node_t), 1);
  if (!(*node)) {
    status = SWITCH_STATUS_NO_MEMORY;
    return status;
  }

  SWITCH_MEMSET(*node, 0x0, sizeof(switch_trie_node_t));
  return status;
}

switch_status_t switch_lpm_trie_create(switch_device_t device,
                                       switch_size_t key_width_bytes,
                                       bool auto_shrink,
                                       switch_lpm_trie_t **trie) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_ASSERT(key_width_bytes <= 64);

  *trie = SWITCH_MALLOC(device, sizeof(switch_lpm_trie_t), 1);
  if (!(*trie)) {
    status = SWITCH_STATUS_NO_MEMORY;
    return status;
  }

  status = switch_trie_node_allocate(device, &(*trie)->root);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  (*trie)->key_width_bytes = key_width_bytes;
  (*trie)->release_memory = auto_shrink;
  (*trie)->num_entries = 0;

  return status;
}

switch_size_t switch_lpm_trie_size(switch_lpm_trie_t *trie) {
  SWITCH_ASSERT(trie != NULL);
  if (!trie) {
    return 0;
  }

  return trie->num_entries;
}

switch_status_t switch_trie_node_destroy(switch_device_t device,
                                         switch_trie_node_t *node) {
  Word_t index = 0;
  Word_t *pnode = NULL;
  Word_t rc_word;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  JLF(pnode, node->PJLarray_branches, index);
  while (pnode != NULL) {
    status = switch_trie_node_destroy(device, ((switch_trie_node_t *)*pnode));
    JLN(pnode, node->PJLarray_branches, index);
  }

  JLFA(rc_word, node->PJLarray_branches);
  JLFA(rc_word, node->PJLarray_prefixes);
  SWITCH_FREE(device, node);

  return status;
}

switch_status_t switch_lpm_trie_destroy(switch_device_t device,
                                        switch_lpm_trie_t *trie) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_trie_node_destroy(device, trie->root);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_FREE(device, trie);
  return status;
}

static inline switch_trie_node_t *switch_trie_next_node_get(
    const switch_trie_node_t *current_node, byte_t byte) {
  Word_t *pnode = NULL;
  JLG(pnode, current_node->PJLarray_branches, (Word_t)byte);
  if (!pnode) return NULL;
  return (switch_trie_node_t *)*pnode;
}

static inline void switch_trie_next_node_set(switch_trie_node_t *current_node,
                                             byte_t byte,
                                             switch_trie_node_t *next_node) {
  Word_t *pnode = NULL;
  JLI(pnode, current_node->PJLarray_branches, (Word_t)byte);
  *pnode = (Word_t)next_node;
  return;
}

static inline int switch_trie_branch_delete(switch_trie_node_t *current_node,
                                            byte_t byte) {
  int rc;
  JLD(rc, current_node->PJLarray_branches, (Word_t)byte);
  return rc;
}

static inline switch_uint16_t switch_trie_prefix_key_get(
    switch_uint32_t prefix_length, byte_t byte) {
  return prefix_length ? (byte >> (8 - prefix_length)) + (prefix_length << 8)
                       : 0;
}

static inline int switch_trie_prefix_insert(switch_trie_node_t *current_node,
                                            switch_uint16_t prefix_key,
                                            const value_t value) {
  Word_t *pvalue;
  int rc;
  JLI(pvalue, current_node->PJLarray_prefixes, (Word_t)prefix_key);
  rc = (*pvalue) ? 1 : 0;
  *pvalue = (Word_t)value;
  return rc;
}

static inline value_t *switch_trie_prefix_ptr_get(
    const switch_trie_node_t *current_node, switch_uint16_t prefix_key) {
  Word_t *pvalue = NULL;
  JLG(pvalue, current_node->PJLarray_prefixes, (Word_t)prefix_key);
  return (value_t *)pvalue;
}

static inline switch_int32_t switch_trie_prefix_delete(
    switch_trie_node_t *current_node, switch_uint16_t prefix_key) {
  int rc = 0;
  JLD(rc, current_node->PJLarray_prefixes, (Word_t)prefix_key);
  return rc;
}

switch_status_t switch_lpm_trie_insert(switch_device_t device,
                                       switch_lpm_trie_t *trie,
                                       const switch_uint8_t *prefix,
                                       switch_size_t prefix_length,
                                       const value_t value) {
  switch_trie_node_t *current_node = trie->root;
  byte_t byte = 0;
  switch_uint16_t prefix_key = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  while (prefix_length >= 8) {
    byte = (byte_t)*prefix;
    switch_trie_node_t *node = switch_trie_next_node_get(current_node, byte);
    if (!node) {
      switch_trie_node_allocate(device, &node);
      node->parent = current_node;
      node->child_id = byte;
      switch_trie_next_node_set(current_node, byte, node);
      current_node->branch_num++;
    }

    prefix++;
    prefix_length -= 8;
    current_node = node;
  }

  if (prefix_length != 0) {
    prefix_key =
        switch_trie_prefix_key_get((unsigned)prefix_length, (byte_t)*prefix);
  }

  if (!switch_trie_prefix_insert(current_node, prefix_key, value))
    current_node->pref_num++;

  trie->num_entries++;
  return status;
}

bool switch_lpm_trie_has_prefix(const switch_lpm_trie_t *trie,
                                const switch_uint8_t *prefix,
                                switch_size_t prefix_length) {
  switch_trie_node_t *current_node = trie->root;
  byte_t byte = 0;
  switch_uint16_t prefix_key = 0;

  while (prefix_length >= 8) {
    byte = (byte_t)*prefix;
    switch_trie_node_t *node = switch_trie_next_node_get(current_node, byte);
    if (!node) return false;
    prefix++;
    prefix_length -= 8;
    current_node = node;
  }

  if (prefix_length != 0) {
    prefix_key =
        switch_trie_prefix_key_get((unsigned)prefix_length, (byte_t)*prefix);
  }

  return (switch_trie_prefix_ptr_get(current_node, prefix_key) != NULL);
}

switch_status_t switch_lpm_trie_lookup(const switch_lpm_trie_t *trie,
                                       const switch_uint8_t *key,
                                       value_t *pvalue) {
  const switch_trie_node_t *current_node = trie->root;
  switch_size_t key_width = trie->key_width_bytes;
  byte_t byte;
  value_t *pdata = NULL;
  switch_uint16_t prefix_key;
  unsigned i;
  switch_status_t status = SWITCH_STATUS_ITEM_NOT_FOUND;

  while (current_node) {
    pdata = switch_trie_prefix_ptr_get(current_node, 0);
    if (pdata) {
      *pvalue = *pdata;
      status = SWITCH_STATUS_SUCCESS;
    }

    if (key_width > 0) {
      byte = (byte_t)*key;
      for (i = 7; i > 0; i--) {
        prefix_key = switch_trie_prefix_key_get((unsigned)i, byte);
        pdata = switch_trie_prefix_ptr_get(current_node, prefix_key);
        if (pdata) {
          *pvalue = *pdata;
          status = SWITCH_STATUS_SUCCESS;
          break;
        }
      }

      current_node = switch_trie_next_node_get(current_node, byte);
      key++;
      key_width--;
    } else {
      break;
    }
  }

  return status;
}

switch_status_t switch_lpm_trie_delete(switch_device_t device,
                                       switch_lpm_trie_t *trie,
                                       const switch_uint8_t *prefix,
                                       switch_size_t prefix_length) {
  switch_trie_node_t *current_node = trie->root;
  byte_t byte = 0;
  switch_uint16_t prefix_key = 0;
  value_t *pdata = NULL;

  while (prefix_length >= 8) {
    byte = (byte_t)*prefix;
    switch_trie_node_t *node = switch_trie_next_node_get(current_node, byte);
    if (!node) return SWITCH_STATUS_FAILURE;

    prefix++;
    prefix_length -= 8;
    current_node = node;
  }

  if (prefix_length != 0) {
    prefix_key =
        switch_trie_prefix_key_get((unsigned)prefix_length, (byte_t)*prefix);
  }

  pdata = switch_trie_prefix_ptr_get(current_node, prefix_key);
  if (!pdata) return SWITCH_STATUS_FAILURE;

  if (trie->release_memory) {
    SWITCH_ASSERT(switch_trie_prefix_delete(current_node, prefix_key) == 1);
    current_node->pref_num--;
    while (current_node->pref_num == 0 && current_node->branch_num == 0) {
      switch_trie_node_t *tmp_node = current_node;
      current_node = current_node->parent;
      if (!current_node) break;
      SWITCH_ASSERT(
          switch_trie_branch_delete(current_node, tmp_node->child_id) == 1);
      SWITCH_FREE(device, tmp_node);
      current_node->branch_num--;
    }
  }

  trie->num_entries--;
  return SWITCH_STATUS_SUCCESS;
}
