/*
 *
 */

#include <iostream>
#include <iomanip>
#include <vector>

#include <TGraph.h>
#include <TAxis.h>
#include <TThread.h>

#include <arpa/inet.h>

#include "mperiod.cxx"
#include "decode.cxx"

#define MAX_CH 8


int gr_draw(TGraph **gr)
{
	bool *fdisp = af->GetEnableCh();
	TCanvas *c = af->GetCanvas();
	c->cd(0);
	int ch_first = -1;
	for (int i = 0 ; i < MAX_CH ; i++) {
		if (fdisp[i]){
			if (ch_first < 0) {
				gr[i]->Draw("al");
				ch_first = i;
			} else {
				gr[i]->Draw("l same");
			}
		}
	}
	if (ch_first < 0) ch_first = 0;

	return ch_first;
}

bool is_change_fdisp(bool *fsrc, bool *fcopy)
{
	bool flag = false;
	for (int i = 0 ; i < MAX_CH ; i++) {
		if (fsrc[i] != fcopy[i])  {
			flag = true;
			fcopy[i] = fsrc[i];
		}
	}
	
	return flag;
}

void reader(void *arg = NULL)
{
	static char cbuf[1024 * 2 * MAX_CH + sizeof(event_header) + sizeof(uint32_t)];
	struct event_header *evh;
	evh = reinterpret_cast<struct event_header *>(cbuf);
	char *evb = cbuf + sizeof(struct event_header);
	static uint32_t evt;
	static std::vector<std::vector<int> > data;
	int event_number = 0;

	int index[4096];
	for (int i = 0 ; i < 4096 ; i++) index[i] = i;

	/*
	bool fdisp[MAX_CH] = {
		true, true, true, true,
		false, false, false, false};
	*/
	TGraph *gr[MAX_CH];
	for (int i = 0 ; i < MAX_CH ; i++) {
		gr[i] = new TGraph();
		gr[i]->SetLineColor(i + 1);
		gr[i]->SetTitle();
		gr[i]->GetXaxis()->SetLimits(0., 1024.);
		gr[i]->SetPoint(0, 0, 512);
		gr[i]->SetMaximum(512. + 512);
		gr[i]->SetMinimum(512. - 512);
	}
	gr[4]->SetLineColor(28);
	


	#if 0
	bool *fdisp = af->GetEnableCh();
	TCanvas *c = af->GetCanvas();
	c->cd(0);
	int ch_first = -1;
	for (int i = 0 ; i < MAX_CH ; i++) {
		if (fdisp[i]){
			if (ch_first < 0) {
				gr[i]->Draw("al");
				ch_first = i;
			} else {
				gr[i]->Draw("l same");
			}
		}
	}
	if (ch_first < 0) ch_first = 0;
	#endif

	int ch_first = gr_draw(gr);
	bool *fdisp = af->GetEnableCh();
	bool fdisp_keep[MAX_CH];
	for (int i = 0 ; i < MAX_CH ; i++) fdisp_keep[i] = fdisp[i];

	mPeriod p;
	p.reset();

	while (true) {
		if (! af->GetRunState()) {
			TThread::Sleep(0, 100000);
			p.reset();
			continue;
		}

		std::cin.read(cbuf, sizeof(struct event_header));
		if (std::cin.eof()) break;
		unsigned int len = ntohl(evh->length);
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

			g_mtx_disp->Lock();

			if (is_change_fdisp(fdisp, fdisp_keep)) ch_first = gr_draw(gr);

			for (int i = 0 ; i < MAX_CH ; i++) {
				if (fdisp[i]) {
					for (unsigned int j = 0 ; j < data[i].size() ; j++) {
						gr[i]->SetPoint(j, index[j], data[i][j]);
					}
				}
			}


			static int prev_scale = 0;
			int scale = af->GetScale();
			if (prev_scale != scale) {
				for (int i = 0 ; i < MAX_CH ; i++) {
					gr[i]->SetMaximum(512. + scale);
					gr[i]->SetMinimum(512. - scale);
				}
				prev_scale = scale;
			}

			int xscale = af->GetXScale();
			gr[ch_first]->GetXaxis()->SetLimits(512. - xscale, 512. + xscale);

			TCanvas *c = af->GetCanvas();
			c->Update();
			g_mtx_disp->UnLock();

			if ((event_number % 10) == 0) {
				std::cout << "\rFPS: " << 1000 / p.tap()
					<< ", Period: " << p.get_mean() << "    ";
			} else {
				p.tap();
			}

		} else {
			std::cerr << "Invalid Header "
				<< ntohl(static_cast<unsigned int>(cbuf[0])) << std::endl;
			break;
		}

		if ((event_number % 100) == 0) std::cout << "Event : " << event_number << std::endl;
		event_number++;
	}

	std::cout << "#D data_reader : out of loop" << std::endl;
	while(true) TThread::Sleep(1, 0);
}

