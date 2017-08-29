#include <readsdoconfig.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return -1;
  }

  SdoConfigParameter_t config_parameter = { 0, 0, NULL };

  /*
   * Read the device configurations from file `filename`
   */
  if (read_sdo_config(argv[1], &config_parameter) != 0) {
    fprintf(stderr, "Error parsing file\n");
    return -1;
  }

  /*
   * Get the node count and the parameter count.
   * This configuration is intended for motordrives only, this means the file
   * parser will fail if the number of parameter objects differ between nodes.
   *
   * To configure a network with different nodes the user of this module needs
   * to take care of this.
   */
  printf("Node Count = %lu; Parameter Count = %lu\n",
         config_parameter.node_count, config_parameter.param_count);

  /*
   * Get the reference of the parameter list to simplify handling.
   * The parameter list is a 2 dimensional array with dimensions
   *
   *   number of nodes X number of parameters
   *
   * The individual nodes are not mentioned separately.
   */
  SdoParam_t **paramlist = config_parameter.parameter;
  if (paramlist == NULL) {
    fprintf(stderr, "Error list of parameter per nodes is empty\n");
    return -1;
  }

  for (size_t i = 0; i < config_parameter.node_count; i++) {
    SdoParam_t *nparam = *paramlist + i;
    if (nparam == NULL) {
      fprintf(stderr, "Error parameter list of node is empty\n");
      return -1;
    }

    for (size_t k = 0; k < config_parameter.param_count; k++) {
      SdoParam_t *p = nparam + k;

      printf("N%zu: 0x%04x:%d = %d\n",
             i, p->index, p->subindex, p->value);
    }
  }

  return 0;
}
