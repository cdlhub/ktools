/*
* Copyright (c)2015 - 2016 Oasis LMF Limited 
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*
*   * Neither the original author of this software nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
* THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*/
/*
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

void doit_old()
{

	fm_profile_old q;
    char line[4096];
    int lineno=0;
	fgets(line, sizeof(line), stdin);
	lineno++;
    while (fgets(line, sizeof(line), stdin) != 0)
    {
		if (sscanf(line, "%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f", &q.policytc_id, &q.calcrule_id, &q.allocrule_id, &q.ccy_id, &q.deductible, &q.limits, &q.share_prop_of_lim, 
              &q.deductible_prop_of_loss, &q.limit_prop_of_loss, &q.deductible_prop_of_tiv, &q.limit_prop_of_tiv, &q.deductible_prop_of_limit) != 12){
           fprintf(stderr, "Invalid data in line %d:\n%s", lineno, line);
           return;
       }
	    else
       {
           fwrite(&q, sizeof(q), 1, stdout);
       }
       lineno++;
    }

}

void doit()
{

	fm_profile_new q;
	char line[4096];
	int lineno = 0;
	fgets(line, sizeof(line), stdin);
	lineno++;
	while (fgets(line, sizeof(line), stdin) != 0)
	{
		if (sscanf(line, "%d,%d,%f,%f,%f,%f,%f,%f,%f,%f", 
			&q.profile_id, &q.calcrule_id,
			&q.deductible1, &q.deductible2, &q.deductible3,
			&q.attachment, &q.limit, &q.share1, &q.share2,
			&q.share3) != 10) {
			fprintf(stderr, "Invalid data in line %d:\n%s", lineno, line);
			return;
		}
		else
		{
			fwrite(&q, sizeof(q), 1, stdout);
		}
		lineno++;
	}

}

void help()
{
	fprintf(stderr, "-h help\n - o old format\n-v version\n");
}

int main(int argc, char* argv[])
{

	int opt;
	bool oldFMProfile = false;

	while ((opt = getopt(argc, argv, "vho")) != -1) {
		switch (opt) {
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'h':
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}
	initstreams();

	if (oldFMProfile == true) {
		doit_old();
	}
	else {
		doit();
	}

	return EXIT_SUCCESS;

}


