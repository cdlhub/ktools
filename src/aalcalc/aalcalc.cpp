
#include "aalcalc.h"

#if defined(_MSC_VER)
#include "../include/dirent.h"
#else
#include <dirent.h>
#endif
#include <math.h>


bool operator<(const period_sidx_map_key& lhs, const period_sidx_map_key& rhs)
{

	if (lhs.summary_id != rhs.summary_id) {
		return lhs.summary_id < rhs.summary_id;
	}
	else {
		if (lhs.sidx != rhs.sidx) {
			return lhs.sidx < rhs.sidx;
		}
		else {
			return lhs.period_no < rhs.period_no;
		}
	}

	
}

bool operator<(const period_map_key& lhs, const period_map_key& rhs)
{
	if (lhs.summary_id != rhs.summary_id) {
		return lhs.summary_id < rhs.summary_id;
	}
	else {
		return lhs.period_no < rhs.period_no;
	}

	//if (lhs.period_no != rhs.period_no) {
	//	return lhs.period_no < rhs.period_no;
	//}
	//else {		
	//	return lhs.summary_id < rhs.summary_id;		
	//}
}


// Load and normalize weigthting table 
// we must have entry for every return period!!!
// otherwise no way to pad missing ones
// Weightings should be between zero and 1 and should sum to one 
void aalcalc::loadperiodtoweigthing()
{
	FILE *fin = fopen(PERIODS_FILE, "rb");
	if (fin == NULL) return;
	Periods p;
	double total_weighting = 0;
	size_t i = fread(&p, sizeof(Periods), 1, fin);
	while (i != 0) {
		total_weighting += p.weighting;
		periodstoweighting_[p.period_no] = (OASIS_FLOAT)p.weighting;
		i = fread(&p, sizeof(Periods), 1, fin);
	}
	// If we are going to have weightings we should have them for all periods
	//	if (periodstowighting_.size() != no_of_periods_) {
	//		fprintf(stderr, "Total number of periods in %s does not match the number of periods in %s\n", PERIODS_FILE, OCCURRENCE_FILE);
	//		exit(-1);
	//	}
	// Weighting already normalzed just split over samples...
	return;
	auto iter = periodstoweighting_.begin();
	while (iter != periodstoweighting_.end()) {
		// iter->second = iter->second / total_weighting; // no need sinece already normalized 
		if (samplesize_ == -1) { 
			fprintf(stderr, "Sample size not initialzed\n"); 
			exit(EXIT_FAILURE);
		}
		if (samplesize_) iter->second = iter->second / samplesize_;   // split weighting over samples
		iter++;
	}
}

void aalcalc::loadoccurrence()
{

	int date_algorithm_ = 0;
	FILE *fin = fopen(OCCURRENCE_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, OCCURRENCE_FILE);
		exit(-1);
	}

	size_t i = fread(&date_algorithm_, sizeof(date_algorithm_), 1, fin);	// discard date algorithm
	i = fread(&no_of_periods_, sizeof(no_of_periods_), 1, fin);
	occurrence occ;
	i = fread(&occ, sizeof(occ), 1, fin);
	while (i != 0) {
		event_count_[occ.event_id] = event_count_[occ.event_id] + 1;
		event_to_period_[occ.event_id].push_back(occ.period_no);
		i = fread(&occ, sizeof(occ), 1, fin);
	}

	fclose(fin);
}
void aalcalc::applyweightings(int event_id, const std::map <int, double> &periodstoweighting, std::vector<sampleslevelRec> &vrec)
{
	if (periodstoweighting.size() == 0) return;
	//double factor = periodstoweighting.size();
	//auto it = event_to_period_.find(event_id);
	//if (it != event_to_period_.end()) {
	//	auto iter = periodstoweighting.find(it->second);
	//	if (iter != periodstoweighting.end()) {
	//		for (int i = 0; i < vrec.size(); i++) {
	//			vrec[i].loss = vrec[i].loss * iter->second * factor;
	//		}
	//	}
	//	else {
	//		// Event not found in periods.bin so no weighting i.e zero 
	//		for (int i = 0; i < vrec.size(); i++) vrec[i].loss = 0;
	//		//	fprintf(stderr, "Event %d not found in periods.bin\n", event_id);
	//		//		exit(-1);
	//	}
	//}
	//else {
	//	fprintf(stderr, "Event ID %d not found\n", event_id);
	//}
}

void aalcalc::applyweightingstomap(std::map<int, aal_rec> &m, int i)
{
	auto iter = m.begin();
	while (iter != m.end()) {
		//iter->second.mean = iter->second.mean * i;
		//iter->second.mean_squared = iter->second.mean_squared * i * i;
		iter++;
	}
}
void aalcalc::applyweightingstomaps()
{
	int i = periodstoweighting_.size();
	if (i == 0) return;
	applyweightingstomap(map_analytical_aal_, i);
	applyweightingstomap(map_sample_aal_, i);
}
int analytic_count = 0;


void aalcalc::do_analytical_calc_end()
{
	auto iter = map_analytical_sum_loss_.begin();
	while (iter != map_analytical_sum_loss_.end()) {
		auto a_iter = map_analytical_aal_.find(iter->first.summary_id);
		if (a_iter != map_analytical_aal_.end()) {
			aal_rec &a = a_iter->second;
			if (a.max_exposure_value < iter->second.max_exposure_value) a.max_exposure_value = iter->second.max_exposure_value;
			a.mean += iter->second.sum_of_loss;
			a.mean_squared += iter->second.sum_of_loss * iter->second.sum_of_loss;
		}
		else {
			aal_rec a;
			a.summary_id = iter->first.summary_id;
			a.type = 1;
			a.max_exposure_value = iter->second.max_exposure_value;
			a.mean = iter->second.sum_of_loss;
			a.mean_squared = iter->second.sum_of_loss * iter->second.sum_of_loss;
			map_analytical_aal_[iter->first.summary_id] = a;
		}
		iter++;
	}
}
void aalcalc::do_sample_calc_end(){

	auto iter = map_sample_sum_loss_.begin();


	while (iter != map_sample_sum_loss_.end()) {
		auto a_iter = map_sample_aal_.find(iter->first.summary_id);
		if (a_iter != map_sample_aal_.end()) {
			aal_rec &a = a_iter->second;
			if (a.max_exposure_value < iter->second.max_exposure_value) a.max_exposure_value = iter->second.max_exposure_value;
			a.mean += iter->second.sum_of_loss;
			a.mean_squared += iter->second.sum_of_loss * iter->second.sum_of_loss;
		}
		else {
			aal_rec a;
			a.summary_id = iter->first.summary_id;
			a.type = 2;
			a.max_exposure_value = iter->second.max_exposure_value;
			a.mean = iter->second.sum_of_loss;
			a.mean_squared = iter->second.sum_of_loss * iter->second.sum_of_loss;
			map_sample_aal_[iter->first.summary_id] = a;
		}
		iter++;
	}

}


void aalcalc::do_sample_calc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec){

	period_sidx_map_key k;
	
	k.summary_id = sh.summary_id;
	auto p_iter = event_to_period_.find(sh.event_id);
	if (p_iter == event_to_period_.end()) return;

	//k.period_no = event_to_period_[sh.event_id];
	//if (k.period_no == 0) return;
	for (auto p : p_iter->second) {
		k.period_no = p;
		for (auto x : vrec) {
			k.sidx = x.sidx;
			auto iter = map_sample_sum_loss_.find(k);
			if (iter != map_sample_sum_loss_.end()) {
				loss_rec &a = iter->second;
				if (a.max_exposure_value < sh.expval) a.max_exposure_value = sh.expval;
				a.sum_of_loss += x.loss;
			}
			else {
				loss_rec l;
				l.sum_of_loss = x.loss;
				l.max_exposure_value = sh.expval;
				map_sample_sum_loss_[k] = l;
			}

		}
	}
	
}
void aalcalc::do_analytical_calc(const summarySampleslevelHeader &sh, double mean_loss)
{
	period_map_key k;	
	k.summary_id = sh.summary_id;
	//k.period_no = event_to_period_[sh.event_id];
	//if (k.period_no == 0) return;
	auto p_iter = event_to_period_.find(sh.event_id);
	if (p_iter == event_to_period_.end()) return;
	for (auto p : p_iter->second) {
		k.period_no = p;
		auto iter = map_analytical_sum_loss_.find(k);
		if (iter != map_analytical_sum_loss_.end()) {
			loss_rec &a = iter->second;
			if (a.max_exposure_value < sh.expval) a.max_exposure_value = sh.expval;
			a.sum_of_loss += mean_loss;
		}
		else {
			loss_rec l;
			l.sum_of_loss = mean_loss;
			l.max_exposure_value = sh.expval;
			map_analytical_sum_loss_[k] = l;
		}
	}
}


void aalcalc::doaalcalc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec, OASIS_FLOAT mean_loss)
{
	do_analytical_calc(sh, mean_loss);
	if (samplesize_) do_sample_calc(sh, vrec);
}

void aalcalc::process_summaryfile(const std::string &filename)
{
	FILE *fin= fopen(filename.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, filename.c_str());
		exit(EXIT_FAILURE);
	}

	int summarycalcstream_type = 0;
	size_t i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, fin);
	int stream_type = summarycalcstream_type & summarycalc_id;

	if (stream_type != summarycalc_id) {
		fprintf(stderr, "%s: Not a summarycalc stream type %d\n", __func__, stream_type);
		exit(-1);
	}
	stream_type = streamno_mask & summarycalcstream_type;
	bool haveData = false;

	if (stream_type == 1) {
		int summary_set = 0;
		i = fread(&samplesize_, sizeof(samplesize_), 1, fin);
		if (i != 0) i = fread(&summary_set, sizeof(summary_set), 1, fin);
		std::vector<sampleslevelRec> vrec;
		summarySampleslevelHeader sh;
		int j = 0;
		OASIS_FLOAT mean_loss = 0;
		while (i != 0) {
			i = fread(&sh, sizeof(sh), 1, fin);
			while (i != 0) {
				haveData = true;
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, fin);
				if (i == 0 || sr.sidx == 0) {					
					auto iter = event_count_.find(sh.event_id);
					if (iter != event_count_.end()) {
						//for (int k = 0; k < event_count_[sh.event_id]; k++) {
							applyweightings(sh.event_id, periodstoweighting_, vrec);
							doaalcalc(sh, vrec, mean_loss);
						//}
					}
					vrec.clear();
					break;
				}
				if (sr.sidx == -1) mean_loss = sr.loss;
				if (sr.sidx >= 0) vrec.push_back(sr);
			}
			haveData = false;
			j++;
		}
		if (haveData == true) {
			applyweightings(sh.event_id, periodstoweighting_, vrec);
			doaalcalc(sh, vrec, mean_loss);
		}
	}
	
	fclose(fin);
}
void aalcalc::outputresultscsv()
{
	if (skipheader_ == false) printf("summary_id,type,mean,standard_deviation,exposure_value\n");
	int p1 = no_of_periods_ ;
	int p2 = p1 - 1;

	for (auto x : map_analytical_aal_) {
		double mean = x.second.mean;
		double mean_squared = x.second.mean * x.second.mean;
		double s1 = x.second.mean_squared - mean_squared / p1;
		double s2 = s1 / p2;
		double sd_dev = sqrt(s2);
		mean = mean / no_of_periods_;
		printf("%d,%d,%f,%f,%f\n", x.first, x.second.type, mean, sd_dev, x.second.max_exposure_value);
	}

	p1 = no_of_periods_ * samplesize_;
	p2 = p1 - 1;

	for (auto x : map_sample_aal_) {
		double mean = x.second.mean / samplesize_;
		double mean_squared = x.second.mean * x.second.mean;
		double s1 = x.second.mean_squared - mean_squared / p1;
		double s2 = s1 / p2;
		double sd_dev = sqrt(s2);
		mean = mean / no_of_periods_;
		printf("%d,%d,%f,%f,%f\n", x.first, x.second.type, mean, sd_dev, x.second.max_exposure_value);
	}

}
void aalcalc::initsameplsize(const std::string &path)
{
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::string s = ent->d_name;
			if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
				s = path + ent->d_name;
				FILE *fin = fopen(s.c_str(), "rb");
				if (fin == NULL) {
					fprintf(stderr, "%s: cannot open %s\n", __func__, s.c_str());
					exit(EXIT_FAILURE);
				}
				int summarycalcstream_type = 0;
				size_t i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, fin);
				i = fread(&samplesize_, sizeof(samplesize_), 1, fin);
				fclose(fin);
				break;
			}

		}
	}
}
void aalcalc::doit(const std::string &subfolder)
{
	
	std::string path = "work/" + subfolder;
	if (path.substr(path.length() - 1, 1) != "/") {
		path = path + "/";
	}
	initsameplsize(path);
	loadoccurrence();
	loadperiodtoweigthing();	// move this to after the samplesize_ variable has been set i.e.  after reading the first 8 bytes of the first summary file

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::string s = ent->d_name;
			if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
				s = path + ent->d_name;				
				process_summaryfile(s);
				//setinputstream(s);
				//processinputfile(samplesize, event_to_periods, maxsummaryid, agg_out_loss, max_out_loss);
			}


		}
		applyweightingstomaps();
		do_sample_calc_end();
		do_analytical_calc_end();
		//outputsummarybin();
		//getnumberofperiods();
		outputresultscsv();
	}
	else {
		fprintf(stderr, "Unable to open directory %s\n", path.c_str());
		exit(-1);
	}	
}
void aalcalc::debug_process_summaryfile(const std::string &filename)
{
	FILE *fin = fopen(filename.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, filename.c_str());
		exit(EXIT_FAILURE);
	}

	int summarycalcstream_type = 0;
	size_t i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, fin);
	int stream_type = summarycalcstream_type & summarycalc_id;

	if (stream_type != summarycalc_id) {
		fprintf(stderr, "%s: Not a summarycalc stream type %d\n", __func__, stream_type);
		exit(-1);
	}
	stream_type = streamno_mask & summarycalcstream_type;
	bool haveData = false;

	if (stream_type == 1) {
		int summary_set = 0;
		i = fread(&samplesize_, sizeof(samplesize_), 1, fin);
		if (i != 0) i = fread(&summary_set, sizeof(summary_set), 1, fin);
		printf("event_id,period_no,summary_id,sidx,loss\n");
		summarySampleslevelHeader sh;
		int j = 0;
		OASIS_FLOAT mean_loss = 0;
		while (i != 0) {
			i = fread(&sh, sizeof(sh), 1, fin);
			while (i != 0) {
				haveData = true;
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, fin);
				if (i == 0) break;
				if (sr.sidx == 0) break;
				auto p_iter = event_to_period_[sh.event_id].begin();
				while (p_iter != event_to_period_[sh.event_id].end()) {
					printf("%d,%d,%d,%d,%f\n", sh.event_id, *p_iter, sh.summary_id, sr.sidx, sr.loss);
				}

			}
			j++;
		}		
	}

	fclose(fin);
}


void aalcalc::debug(const std::string &subfolder)
{
	loadoccurrence();
	std::string path = "work/" + subfolder;
	if (path.substr(path.length() - 1, 1) != "/") {
		path = path + "/";
	}

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::string s = ent->d_name;
			if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
				s = path + ent->d_name;
				debug_process_summaryfile(s);
			}
		}
	}
	else {
		fprintf(stderr, "Unable to open directory %s\n", path.c_str());
		exit(-1);
	}
}