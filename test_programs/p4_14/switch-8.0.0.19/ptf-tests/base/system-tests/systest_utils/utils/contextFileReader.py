import os, json, argparse
import constants

# TODO: Context.json to what i get from siwtchapi  table
# TODO: Match context.json with what is given by thrift call


class ContextFileReader(object):
    def __init__(self, file):
        """ Class to read context.json and some helper functions """
        # Assumption is that the file exisits and its validity is already done.
        # before calling this class
        self.file = file
        self.context = None
        with open(self.file) as fd:
            self.context = json.loads(fd.read())
        # Build the dic based on what you can infer

    def __repr__(self):
        return 'ContextFileReader("%s")' % self.file

    def __str__(self):
        return 'ContextFileReader object for file : "%s"' % self.file

    def getCompilerVersion(self):
        """ Return Compiler version """
        return str(self.context['compiler_version']) if self.context.has_key(
            'compiler_version') else None

    def getSchemaVersion(self):
        """ return schema_version version """
        return str(self.context['schema_version']) if self.context.has_key(
            'schema_version') else None

    def getProgramName(self):
        """ return program name """
        return str(self.context['program_name']) if self.context.has_key(
            'program_name') else None

    def getBuildDate(self):
        """ Return Build date """
        return str(self.context['build_date']) if self.context.has_key(
            'build_date') else None

    def getNumTables(self):
        """ Return the number of tables """
        return len(
            self.context['tables']) if self.context.has_key('tables') else None

    def getTotalTableSize(self):
        """ return the total table size """
        return sum([int(table['size']) for table in self.context['tables']
                    ]) if self.context.has_key('tables') else None

    def getTableSizeByName(self, tableName):
        """ Return the table size corresponding to table name """
        if self.context.has_key('tables'):
            for table in self.context['tables']:
                if str(table['name']) == tableName:
                    return int(table['size'])
        return None

    def getAllTables(self):
        """ Return (tableName, size) list """
        if self.context.has_key('tables'):
            return [(str(table['name']), int(table['size']))
                    for table in self.context['tables']]
        return []

    def _getTableBasedOntype(self, tableType):
        """ Get Table based on type """
        if self.context.has_key('tables'):
            return [
                table for table in self.context['tables']
                if str(table['table_type']) == tableType
            ]
        return []

    def getTablesTypeMatch(self):
        """ Get all tables  of type action """
        return self._getTableBasedOntype(tableType='match')

    def getTablesTypeAction(self):
        """ Get all tables of type action """
        return self._getTableBasedOntype(tableType='action')

    def getTablesTypeStatistics(self):
        """ Get all tables of type statistics """
        return self._getTableBasedOntype(tableType='statistics')

    def getTablesTypeSelection(self):
        """ Get all tables of type selection"""
        return self._getTableBasedOntype(tableType='selection')

    def getIngressParserRules(self):
        """ Get all ingress parser rules """
        if self.context.has_key('parser'):
            if self.context['parser'].has_key('ingress'):
                return self.context['parser']['ingress']
        return []

    def getEgressParserRules(self):
        """ Get all ingress parser rules """
        if self.context.has_key('parser'):
            if self.context['parser'].has_key('egress'):
                return self.context['parser']['egress']
        return []

    def _filterTableByName(self, name):
        """ General filter , parital match """
        if self.context.has_key('tables'):
            return [
                table for table in self.context['tables']
                if name in table['name']
            ]
        return []

    def getMaxTableSizeBasedOnFeatureName(self, featureName):
        """ Try to get the max table size based on fearure \
			This may not be accurate , Will try to get a best estimate"""
        table_list = self._filterTableByName(name=featureName)
        size_list = [int(table['size']) for table in table_list]
        if size_list != []:
            return max(size_list)
        return 0

    def getMinTableSizeBasedOnFeatureName(self, featureName):
        """ Try to get the max table size based on fearure \
            This may not be accurate , Will try to get a best estimate"""
        table_list = self._filterTableByName(name=featureName)
        size_list = [int(table['size']) for table in table_list]
        if size_list != []:
            return min(size_list)
        return 0

    def printTablesInfo(self):
        """ Print table info """
        print "{}".format("-" * 80)
        print "{:>50} : {:>10}".format('Program Name', self.getProgramName())
        print "{:>50} : {:>10}".format('Toatl number of tables',
                                       self.getNumTables())
        print "{:>50} : {:>10}".format('Total entries',
                                       self.getTotalTableSize())
        print "{:>50} : {:>10}".format('Tables of type action',
                                       len(self.getTablesTypeAction()))
        print "{:>50} : {:>10}".format('Tables of type match',
                                       len(self.getTablesTypeMatch()))
        print "{:>50} : {:>10}".format('Tables of type statistics',
                                       len(self.getTablesTypeStatistics()))
        print "{}".format("-" * 80)
        for tableName, size in self.getAllTables():
            print "{:>50} : {:>10}".format(tableName, size)
        print "{}".format("-" * 80)

    def printMaxTableSizeBasedOnFeatures(self):
        """ Print Max table we see based on the profile """
        print "{}".format("-" * 80)
        for name in constants.switch_feature_list:
            print "{:>50} : {:>10}".format(
                name, self.getMaxTableSizeBasedOnFeatureName(name))
        print "{}".format("-" * 80)

    def get_all_switch_feature_to_table_dict(self):
        """ Build a dict feature, table_size """
        return {
            f: self.getMaxTableSizeBasedOnFeatureName(f)
            for f in constants.switch_feature_list
        }

    def get_all_l2_feature_to_table_dict(self):
        """ return a dict for l2 feature to table size """
        return {
            f: self.getMaxTableSizeBasedOnFeatureName(f)
            for f in constants.switch_l2_feature_list
        }

    def get_all_l3_feature_to_table_dict(self):
        """ return a dict for l2 feature to table size """
        return {
            f: self.getMaxTableSizeBasedOnFeatureName(f)
            for f in constants.switch_l3_feature_list
        }


if __name__ == "__main__":
    desc = 'Read the context.json file and print table info'
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument(
        '-f',
        '--file',
        required=True,
        action='store',
        type=str,
        help="location of context.json file")
    args = parser.parse_args()

    if not os.path.exists(args.file):
        print "File : %s doesnot exist" % (args.file)
        exit(1)

    context = ContextFileReader(args.file)
    context.printTablesInfo()
    context.printMaxTableSizeBasedOnFeatures()
    exit(0)

__all__ = ['ContextFileReader']
