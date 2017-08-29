/*
 * readsdoconfig.h
 *
 * Read device configuration for the SDO transfers from CSV file.
 *
 * Frank Jeschke <fjeschke@synapticon.com>
 *
 * 2017 Synapticon GmbH
 */

#ifndef READSDOCONFIG_H
#define READSDOCONFIG_H

#include <stdint.h>
#include <stdlib.h>

/**
 * \brief Structure describing a individual parameter
 */
typedef struct {
  uint16_t index;      ///< Index of the associated object in the object dictionary
  uint8_t  subindex;   ///< Subindex of this object
  uint32_t value;      ///< Value of the container /* FIXME hold values of arbitrary size! */
  size_t   bytecount;  /* for this value I need access to the object dictionary! FIXME may remove from here! */
} SdoParam_t;

typedef struct {
  size_t node_count;       ///< Number of nodes in the config parameters file, this value needs to be checked against the real number of nodes on the bus
  size_t param_count;      ///< Number of configuration parameters, aka non commented lines in the config file
  SdoParam_t **parameter;  ///< array of node_count x param_count of configuration parameters, for every node is a list of SdoParam_t objects.
} SdoConfigParameter_t;

/**
 * \brief Function to read configuration file and parse the content
 *
 * \param path     Filename with full path of the file to process
 * \param params   Pointer to a \c SdoConfigParameter_t object
 * \return         0 if no error
 */
int read_sdo_config(const char *path, SdoConfigParameter_t *params);

#endif /* READSDOCONFIG_H */
