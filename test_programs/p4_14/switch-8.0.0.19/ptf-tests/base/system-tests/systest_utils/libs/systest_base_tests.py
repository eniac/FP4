
import os, sys

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../'))
sys.path.append(os.path.join(this_dir, '../../../api-tests/'))

import api_base_tests

class systestBaseTests(api_base_tests.ThriftInterfaceDataPlane):

    def setUp(self):
        """ Setup for Systest """
        #TODO: Do system test logging part here.
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)


    def tearDown(self):
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)
