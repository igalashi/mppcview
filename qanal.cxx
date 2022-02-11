#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>

#include "TSystem.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "TThread.h"

#include "decode.cxx"

const int MAX_CH = 8;
const int MAX_WAVE = 1024;

int scan_min(std::vector<int> &data)
{
	int min = 1024;
	for (unsigned int i = 0 ; i < data.size() ; i++) {
		int val = data[i] & 0x3ff;
		//bool otr = (data[i] & 0x400 != 0);
		if (val < min) min = val;
	}

	return min;
}

double scan_min3(std::vector<int> &data)
{
	int min = 1024;
	int min_index = 0;
	for (unsigned int i = 0 ; i < data.size() ; i++) {
		int val = data[i] & 0x3ff;
		//bool otr = (data[i] & 0x400 != 0);
		if (val < min) {
			min = val;
			min_index = i;
		}
	}

	int min1 = (data[min_index] & 0x3ff);
	int min2 = (min_index == 0) ? (data[0] & 0x3ff) : (data[min_index - 1] & 0x3ff);
	int min3 = (min_index == MAX_WAVE - 1) ? (data[MAX_WAVE - 1] & 0x3ff) : (data[min_index + 1] & 0x3ff);

	double retval = (
		static_cast<double>(min1) + static_cast<double>(min2)
		+ static_cast<double>(min3)) / 3.0;

	return retval;
}

double scan_min_integ(std::vector<int> &data)
{
	int min = 1024;
	int min_index = 0;
	for (unsigned int i = 0 ; i < data.size() ; i++) {
		int val = data[i] & 0x3ff;
		//bool otr = (data[i] & 0x400 != 0);
		if (val < min) {
			min = val;
			min_index = i;
		}
	}

	double integ;
	if ((min_index > 4) && (min_index < 1018)) {
		integ = 0;
		for (int i = min_index - 3 ;  i < min_index + 8 ; i++) {
			integ += 512 - data[i] + 10;
		}
		integ = integ / 2.0;

	} else {
		integ = -1024;
	}

	return integ;
}

double region_integ(std::vector<int> &data)
{
	const int low = 508;
	const int high = 519;
	//const int high = 530;

	double integ;
	integ = 0;
	for (int i = low ;  i < high + 1 ; i++) {
		integ += 512 - data[i] + 10;
	}
	integ = integ / 2.0;

	return integ;
}


static TH2F* h2adc[MAX_CH];
static TH1F* h1peak[MAX_CH];

int hist_init()
{
	for (int i = 0 ; i < MAX_CH ; i++) {
		h2adc[i] = new TH2F(Form("adc%02d", i), Form("ADC %02d", i),
			MAX_WAVE, 0, MAX_WAVE, 1024, 0, 1024);
		h1peak[i] = new TH1F(Form("peak%02d", i), Form("PEAK %02d", i),
			1024, 0, 1024);
	}

	return 0;
}

int hist_fill(std::vector<std::vector<int> > &data)
{
	#if 0
	std::cout << std::dec;
	for (unsigned int i = 0 ; i < data.size() ; i++) {
		std::cout << "# " << i << " : " << data[i].size() << " : " ;
		for (int j = 0 ; j < 8 ; j++) std::cout << " " << data[i][j];
		std::cout << std::endl;
	}
	#else
	// static unsigned int prescale = 0;
	// if ((prescale++ % 10) == 0) std::cout << "." << std::flush;
	#endif

	for (unsigned int i = 0 ; i < data.size() ; i++) {
		for (unsigned int j = 0 ; j < data[i].size() ; j++) {
			int val;
			if ((data[i][j] & 0x400) == 0x400) {
				val = 1023;
			} else {
				val = data[i][j] & 0x3ff;
			}

			h2adc[i]->Fill(j, val, 1.0);
		}
		//h1peak[i]->Fill(scan_min(data[i]), 1.0);
		//h1peak[i]->Fill(scan_min3(data[i]), 1.0);
		//h1peak[i]->Fill(scan_min_integ(data[i]), 1.0);
		h1peak[i]->Fill(region_integ(data[i]), 1.0);
	}

	return 0;
}

#if 0
void reader(void *arg = NULL)
{
	static char cbuf[1024];
	std::vector<int> data;
	int event_number = 0;
	while (true) {
		std::cin.read(cbuf, sizeof(struct event));
		if (std::cin.eof()) break;
		if (decode_data(cbuf, data) > 0) {
			hist_fill(data);
		} else {
			std::cerr << "Invalid Header "
				<< ntohl(static_cast<unsigned int>(cbuf[0])) << std::endl;
			break;
		}
		if ((event_number % 100) == 0) std::cout << "Event : " << event_number << std::endl;
		event_number++;
	}
}
#endif

void reader(void *arg = NULL)
{
	static char cheader[sizeof(struct event_header)];
	struct event_header *evh;
	evh = reinterpret_cast<struct event_header *>(cheader);
	static uint32_t evt;
	static std::vector<std::vector<int> > data;
	int event_number = 0;
	char *cbuf = nullptr;
	unsigned int max_len = 0;

	while (true) {

		std::cin.read(cheader, sizeof(struct event_header));
		if (std::cin.eof()) break;
		unsigned int len = ntohl(evh->length);

		if (max_len < len) {
			if (cbuf != nullptr) delete cbuf;
			max_len = len;
		}
		if (cbuf == nullptr) {
			cbuf = new char[
				sizeof(struct event_header) + len + sizeof(uint32_t)
			];
		}
		memcpy(cbuf, cheader, sizeof(struct event_header));
		char *evb = cbuf + sizeof(struct event_header);

		std::cin.read(evb, len);
		std::cin.read(reinterpret_cast<char *>(&evt), sizeof(uint32_t));

		#if 0
		unsigned int magic = ntohl(evh->magic);
		unsigned int trig = ntohl(evh->trigger);
		std::cout << "#D 0x" << std::hex << std::setw(8) << magic
			<< " len: " << std::dec << len
			<< " Trig: " << std::dec << trig
			<< " Trailer: " << std::hex << evt << std::endl;
		#endif
		
		if (decode_data(cbuf, data) > 0) {
			hist_fill(data);
		} else {
			std::cerr << "Invalid Header "
				<< ntohl(static_cast<unsigned int>(cbuf[0])) << std::endl;
			break;
		}

		if ((event_number % 100) == 0)
			std::cout << "\r Event : " << event_number << "  ";
			// << std::endl;
		event_number++;
	}

}


#if 0
int ta_argc = 1;
char *ta_argv[] = {const_cast<char *>("App")};
static TApplication *app = new TApplication("App", &ta_argc, ta_argv);
static TCanvas *c1 = new TCanvas("c1", "Online Display");

void updater(void *arg = NULL)
{
	while (true) {
		TThread::Sleep(1, 0);
		c1->cd(1)->Modified();
		c1->cd(2)->Modified();
		c1->Update();
	}
}
#endif


int main(int argc, char* argv[])
{

	gApplication = new TApplication("App", &argc, argv);
	TCanvas *c1 = new TCanvas("c1", "Display", 600, 800);

	#if 0
	for (int i = 1 ; i < argc ; i++) {
		if (strncmp(argv[i], "--no-exit", 9) == 0) no_exit = 1;
	}
	#endif

	//std::cout << "sizeof(struct event_header) = " << sizeof(struct event_header) << std::endl;

	int ch = 0;
	for (int i = 1 ; i < argc ; i++) {
		ch = atoi(argv[i]);
	}


	hist_init();

	c1->Divide(1, 2);
	c1->cd(1);
	c1->cd(1)->SetGrid();
	h2adc[ch]->Draw("colz");
	c1->cd(2);
	c1->cd(2)->SetGrid();
	h1peak[ch]->Draw();
	c1->Update();
	c1->Flush();

	#if 1
	TThread threader("Reader", &reader, NULL);
	threader.Run();
	/*
	TThread thupdater("Updater", &updater, NULL);
	thupdater.Run();
	*/
	#else 
	reader();
	#endif

	#if 1
	TTimer timer(500);
	//timer.SetCommand("printf(\"Hello\");");
	//timer.SetCommand("c1->Update();printf(\"hello\")");
	timer.SetCommand("c1->cd(1)->Modified();c1->cd(2)->Modified();c1->Update();");
	timer.Start();
	#endif

	//app->Run();
	gApplication->Run();
	#if 0
	while (true) {
		TThread::Sleep(0, 100000000);
		gSystem->ProcessEvents();
		std::cout << "x" << std::flush;
	}
	#endif

	return 0;
}
