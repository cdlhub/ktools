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


#ifndef AALCALC_H_
#define AALCALC_H_

#include "../include/oasis.h"

#include <string>
#include <map>
#include <vector>
struct aal_rec_vec {
	std::vector<float> v;
	double max_exposure_value;
};
// key for analytic data
struct period_map_key {
	int summary_id;
	int period_no;
};
// key for sample data
struct period_sidx_map_key {
	int summary_id;
	int period_no;
	int sidx;
};

struct loss_rec {
	double sum_of_loss;
	//double sum_of_loss_squared;
	double max_exposure_value;
};

struct welford_state_info {
	double total_samples = 0;
	double I = 0;
	double K = 0;
	double L = 0;
};

struct loss_rec_w {
	welford_state_info w;
	double max_exposure_value;
};


struct aal_rec_w {
	int summary_id;
	int type;
	double mean;
	double mean_squared;
	double max_exposure_value;
};

//bool operator<(const period_sidx_map_key& lhs, const period_sidx_map_key& rhs);
bool operator<(const period_map_key& lhs, const period_map_key& rhs);

class aalcalc {
private:
	std::map<int, int> event_count_;	// count of events in occurrence table used to create cartesian effect on event_id
	std::map<int, std::vector<int>> event_to_period_;	// Mapping of event to period no
	int no_of_periods_ = 0;
	int samplesize_ = -1;
	std::map<int, aal_rec> map_analytical_aal_;
	std::map<int, aal_rec> map_analytical_aal_w_;
	std::map<int, aal_rec> map_sample_aal_;
	std::map<period_sidx_map_key, loss_rec > map_sample_sum_loss_;
	std::map<period_map_key, loss_rec > map_analytical_sum_loss_;
	std::map<period_map_key, loss_rec_w > map_analytical_sum_loss_w_;
	std::map <int, double> periodstoweighting_;
	bool skipheader_ = false;
// private functions
	void loadoccurrence();
	void initsameplsize(const std::string &path);
	void loadperiodtoweigthing();
	void process_summaryfile(const std::string &filename);
	void process_summaryfilew(const std::string &filename);
	void debug_process_summaryfile(const std::string &filename);
	void do_analytical_calc(const summarySampleslevelHeader &sh, double mean_loss);
	void do_analytical_calcw(const summarySampleslevelHeader &sh, double mean_loss);
	void do_analytical_calc_end();	
	void do_analytical_calc_endw();
	void do_sample_calcw(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec);
	void do_sample_calc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec);
	void do_sample_calc_end();
	void do_sample_calc_endw();
	void doaalcalc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec, OASIS_FLOAT mean_loss);
	void doaalcalcw(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec, OASIS_FLOAT mean_loss);
	void applyweightings(int event_id, const std::map <int, double> &periodstoweighting, std::vector<sampleslevelRec> &vrec) ;
	void applyweightingstomap(std::map<int, aal_rec> &m, int i);
	void applyweightingstomaps();
	void outputresultscsv();
public:
	aalcalc(bool skipheader) : skipheader_(skipheader) {};
	void doit(const std::string &subfolder);
	void doitw(const std::string &subfolder);	// calcuate using welford method 
	void debug(const std::string &subfolder);
};

#endif  // AALCALC_H_