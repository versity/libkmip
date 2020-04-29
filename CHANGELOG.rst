=========
Changelog
=========

.. _v0.2.1: https://github.com/versity/libkmip.git

0.2.1 - April 29, 2020
~~~~~~~~~~~~~~~~~~~~~~

* Support Versity Development
  - Add support for creation of RPM pagakes under 'mock'.
  - Add additional functions to libkmip that allow information
    from the context to made available for diagnositc purposes.
  - Additional program, kmip-get, for use at customer sites to
    verify that the 'libkmip' library is compatible with their
    KMIP server.
  - Changes to support CentOS 7
    + Support for OpenSSL 1.0
    + Issues with gcc not properly handling C11 {0} initializer.

.. _v0.2:

0.2 - July 12, 2019
~~~~~~~~~~~~~~~~~~~

* Add the BSD 3-clause license to the library
* Add KMIP 2.0 attributes
* Add deep copy utilities for all attribute types
* Upgrade Create support to enable KMIP 2.0 encodings
* Upgrade the unit test suite to use intelligent test tracking
* Upgrade the linked list to support enqueue and double linkage
* Fix an implicit infinite loop in the test suite application
* Fix a usage issue when passing no args to the demo applications
* Fix Travis CI config to redirect OpenSSL install logs to a file 

.. _v0.1:

0.1 - November 15, 2018
~~~~~~~~~~~~~~~~~~~~~~~

* Initial release
* Add encoding/decoding support for Symmetric/Public/Private Keys
* Add encoding/decoding support for Create/Get/Destroy operations
* Add KMIP 1.0 - 1.4 support for all supported KMIP structures
* Add an OpenSSL BIO client to securely connect to KMIP servers
* Add demo applications that show how to use the client API
* Add a unit test suite that covers the encoding/decoding library
* Add library documentation built and managed by Sphinx
* Add a Makefile that can build/install static/shared libraries

.. _`master`: https://github.com/OpenKMIP/libkmip/

