Implementation of reading SDO configuration from file
=====================================================

To make the configuration of CiA402 drives easier, the configuration is read
with the help of the library libreadsdoconfig, from a configuration file. The
configuration is in a simple comma separated values file.

The formatting of the configuration file is defined as ::

  # this is a comment line
  <index>,<subindex>,<value1>,<value2>,...,<valueN>

The special character `#` is used for commands.  Lines starting with `#` are
ignored, if a `#` occures somewhere within a line the rest of the line is also
ignored. After the necessary entries `<index>` and `<subindex>` a arbitrary
number of values follow. The first value is for the first node (usually axis),
the second value for the second node, and so on.

It is necessary that the number of nodes is the same for every line in the file.

The master software must check the number of slaves connected with the number
of node values in the file. If the number don't match a error must be
announced.

Since the SDO configuration is only used for devices with a object dictionary
(here CiA402 drives) the calling software needs to take care of every non drive
nodes on the bus. For example, if a I/O EtherCAT node is in the first position
on the bus, the EtherCAT drives will start at position `1` and the application
has to take care the drive `0` is mapped to EtherCAT node `1`.
