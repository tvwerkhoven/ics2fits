// Compile with 
// g++ ics2fits.cc io.cc -o ics2fits -I/sw/include -L/sw/lib -lics -lz -lcfitsio -Wall -g

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <libics.h>
#include <fitsio.h>
#include "io.h"
#include "ics2fits.h"

int main(int argc, char *argv[]) {
  // Init
  Io *io = new Io(4);
  
  // Parse arguments
  if (argc < 2)
    io->msg(IO_ERR | IO_FATAL, "Syntax: %s <filename.ics> [fitsfile.fits]", argv[0]);
  
  // Load data
  ICS* ip;
  Ics_DataType dt;
  int ndims;
  size_t dims[ICS_MAXDIM];
  size_t bufsize;
  void* buf;
  Ics_Error retval;

  retval = IcsOpen (&ip, argv[1], "r");
  if (retval != IcsErr_Ok)
    io->msg(IO_ERR | IO_FATAL, "Error, could not open file.");

  IcsGetLayout (ip, &dt, &ndims, dims);
  bufsize = IcsGetDataSize (ip);
  buf = malloc (bufsize);
  if (buf == NULL)
    io->msg(IO_ERR | IO_FATAL, "Error, could not allocate memory.");
    
  retval = IcsGetData (ip, buf, bufsize);
  if (retval != IcsErr_Ok)
    io->msg(IO_ERR | IO_FATAL, "Error, could not read data.");
  
  retval = IcsClose (ip);
  if (retval != IcsErr_Ok)
    io->msg(IO_WARN, "Warning, could not close file.");
  
  // Print metadata
  
  io->msg(IO_INFO, "Got data at %p, ndims: %d, dims:", buf, ndims);
  io->msg(IO_INFO | IO_NOID, "dim[0] = %d", dims[0]);
  for (int i=1; i<ndims; i++)
    io->msg(IO_INFO | IO_NOID, ", dim[%d] = %d", i, dims[i]);
  io->msg(IO_INFO | IO_NOID, "\n");
  
  // Save as fits

  fitsfile *fptr;
  int status = 0, ii, chan;
  long fpixel = 1, naxis = ndims, nelements = 1;
  long naxes[naxis];
  for (int i=0; i<ndims; i++) {
    naxes[i] = dims[i];
    nelements *= naxes[i];
  }
	
	io->msg(IO_INFO, "nelements = %ld", nelements);

  if (argc >= 3)
    fits_create_file(&fptr, argv[2], &status);
  else
    fits_create_file(&fptr, "ics2fits-out.fits", &status);

  // First dimension is the # of channels:
	int nchan = dims[0];
  naxes[ndims-1] = nchan;
	// The other dimensions are spatial
  for (int i=0; i<ndims-1; i++)
    naxes[i] = dims[i+1];
  
  if (dt == Ics_uint16) {
    uint16_t *tmp = (uint16_t *) malloc(nelements * sizeof(uint16_t));
    for (chan=0; chan < nchan; chan++)
      for (ii=0; ii < (nelements/nchan); ii++)
        tmp[ii + chan*nelements/nchan] = ((uint16_t *) buf)[ii * nchan + chan];
    
    fits_create_img(fptr, USHORT_IMG, naxis, naxes, &status);
    fits_write_img(fptr, TUSHORT, fpixel, nelements, tmp, &status);    
  }
  else {
    io->msg(IO_ERR | IO_FATAL, "Unsupported datatype, cannot continue.");
  }
  
  fits_close_file(fptr, &status);
  fits_report_error(stderr, status);
  return (status);
}

