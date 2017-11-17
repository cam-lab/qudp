#-------------------------------------------------------------------------------
#
#     qudp project build construction script
#

import os

#-------------------------------------------------------------------------------
#
#     General
#
ProductName = 'qudp'

Import('qtEnv')
env = qtEnv.Clone()
env['PRODUCT_NAME'] = ProductName

#-------------------------------------------------------------------------------
#
#     Build hierarchy
#
SConscript('src/SConscript',
            exports = 'env',
            variant_dir = '#build/%s/%s' % (env['VARIANT'], ProductName), duplicate = 0)

SConscript('test/SConscript',
            exports = 'env',
            variant_dir = '#build/%s/%s' % (env['VARIANT'], os.path.join(ProductName, 'test') ), duplicate = 0)


#-------------------------------------------------------------------------------

