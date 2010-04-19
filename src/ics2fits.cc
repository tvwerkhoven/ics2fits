/*
 ics2fits.cc -- convert ics file to fits.
 
 Can be manually compiled with something like:
 $ g++ ics2fits.cc io.cc -o ics2fits -I/sw/include -L/sw/lib -lics -lz -lcfitsio -Wall -g

 Copyright (C) 2010 Tim van Werkhoven <t.i.m.vanwerkhoven@xs4all.nl>
 
 This file is part of ics2fits.
 
 ics2fits is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.
 
 ics2fits is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with ics2fits.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <inttypes.h>
#include <getopt.h>

#include <libics.h>
#include <fitsio.h>
#include "io.h"
#include "ics2fits.h"

using namespace std;

// Some global variables
string execname;
string infile;
string outfile;
Io *io;

// From getopt.h
extern int optind;

void show_version() {
	printf("ics2fits (%s version %s, built %s %s)\n", PACKAGE_NAME, PACKAGE_VERSION, __DATE__, __TIME__);
	printf("Copyright (c) 2010 Tim van Werkhoven <T.I.M.vanWerkhoven@xs4all.nl>\n");
	printf("\nics2fits comes with ABSOLUTELY NO WARRANTY. This is free software,\n"
				 "and you are welcome to redistribute it under certain conditions;\n"
				 "see the file COPYING for details.\n");
}


void show_clihelp(bool error = false) {
	if (error)
		io->msg(IO_ERR | IO_FATAL, "Try '%s --help' for more information.", execname.c_str());
	else {
		printf("Usage: %s [option] <input.ics> [output.fits] ...\n\n", execname.c_str());
		printf("  -v, --verb[=LEVEL]   Increase verbosity level or set it to LEVEL.\n"
					 "  -q,                  Decrease verbosity level.\n"
					 "  -h, --help           Display this help message.\n"
					 "      --version        Display version information.\n\n");
		printf("Report bugs to Tim van Werkhoven <T.I.M.vanWerkhoven@xs4all.nl>.\n");
	}
}

int parse_args(int argc, char *argv[]) {
	int r, option_index = 0;
	execname = argv[0];
	
	static struct option const long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"version", no_argument, NULL, 1},
		{"verb", required_argument, NULL, 2},
		{NULL, 0, NULL, 0}
	};
	
	while((r = getopt_long(argc, argv, "hvq", long_options, &option_index)) != EOF) {
		switch(r) {
			case 0:
				break;
			case 'h':
				show_clihelp(false);
				exit(0);
			case 1:
				show_version();
				exit(0);
			case 'q':
				io->decVerb();
				break;
			case 'v':
				io->incVerb();
				break;
			case 2:
				if(optarg)
					io->setVerb((int) atoi(optarg));
				else {
					show_clihelp(true);
				}
				break;
			case '?':
				show_clihelp(true);
			default:
				break;
		}
	}
	
	return optind;
}


int main(int argc, char *argv[]) {
  // Init
	int r=0;
  io = new Io(2);
	
  // Parse arguments
	r = parse_args(argc, argv);	
  argc -= r; argv += r;

	// Get input file or die
	if (argc >= 1) infile = argv[0];
	else show_clihelp(true);
	
	// Get output file or make one up
	if (argc >= 2) outfile = argv[1];
	else outfile = infile + ".fits";
	
  // Init ICS variables
  ICS* ip;
	const char *errtxt;
  Ics_DataType dt;
  int ndims;
  size_t dims[ICS_MAXDIM];
  size_t bufsize;
  void* buf;
  Ics_Error retval;

	// Open ICS file
	io->msg(IO_INFO, "Reading file '%s'...", infile.c_str());
  retval = IcsOpen(&ip, infile.c_str(), "r");
  if (retval != IcsErr_Ok) {
		errtxt = IcsGetErrorText(retval); 
		io->msg(IO_ERR | IO_FATAL, "Could not open file '%s': %s.", infile.c_str(), errtxt);
	}

  IcsGetLayout (ip, &dt, &ndims, dims);
  bufsize = IcsGetDataSize (ip);
  buf = malloc (bufsize);
	
  retval = IcsGetData (ip, buf, bufsize);
  if (retval != IcsErr_Ok) {
		errtxt = IcsGetErrorText(retval); 
		io->msg(IO_ERR | IO_FATAL, "Could not read data: %s.", errtxt);
	}
  
  retval = IcsClose (ip);
  
  // Print metadata
  io->msg(IO_XNFO, "Got data at %p, ndims: %d, dims:", buf, ndims);
  io->msg(IO_XNFO | IO_NOID, "dim[0] = %d", dims[0]);
  for (int i=1; i<ndims; i++)
    io->msg(IO_XNFO | IO_NOID, ", dim[%d] = %d", i, dims[i]);
  io->msg(IO_XNFO | IO_NOID, "\n");
  
  // Save as fits

  fitsfile *fptr;
  int status = 0, ii, chan;
  long fpixel = 1, naxis = ndims, nelements = 1;
  long naxes[naxis];
  for (int i=0; i<ndims; i++) {
    naxes[i] = dims[i];
    nelements *= naxes[i];
  }
	
	io->msg(IO_XNFO, "nelements = %ld", nelements);

	io->msg(IO_INFO, "Saving file to '%s'.", outfile.c_str());
	
	if (fits_create_file(&fptr, outfile.c_str(), &status)) {
		io->msg(IO_ERR, "Could not create file for writing.");
		fits_report_error(stderr, status);
		return status;
	}

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
		if (status) {
			io->msg(IO_ERR, "Could not write fits image.");
			fits_report_error(stderr, status);
			return status;
		}
  }
  else {
    io->msg(IO_ERR | IO_FATAL, "Unsupported datatype, cannot continue.");
  }
  
  fits_close_file(fptr, &status);
  return status;
}

