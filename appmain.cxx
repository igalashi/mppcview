// example.C

#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <RQ_OBJECT.h>
#include <TApplication.h>
#include <TLine.h>
#include <TMath.h>
#include <TThread.h>
#include <TMutex.h>
#include <TSystem.h>


TMutex *g_mtx_disp;

#include "appframe.h"

static TCanvas *c1;
static AppFrame *af;


#include "datareader.cxx"

#if 0
static TLine *l = new TLine();
void drawline()
{
	static double omega = 0;
	const double domega = TMath::Pi() * 2.0 / 500;
	const double r = 0.3;
	const double x_offset = 0.5;
	const double y_offset = 0.5;

	l->DrawLine(
		r * cos(omega) + x_offset,
		r * sin(omega) + y_offset,
		r * cos(omega + domega) + x_offset,
		r * sin(omega + domega) + y_offset);
	omega += domega;
}
#endif

void updater(void *arg = NULL)
{
	while (true) {
		TThread::Sleep(0, 100000);
		#if 0
		if (af->GetRunState()) {
			c1->Clear();
			//drawline();
			c1->Update();
		}
		#endif
	}
}

int drop_stderr()
{
	int fd = open("/dev/null", O_WRONLY);
	if (fd >= 0) {
		int status = dup2(fd, 2);
		return status;
	} else {
		return fd;
	}
}


int main(int argc, char* argv[])
{
	for (int i = 1 ; i < argc ; i++) {
		std::string sargv(argv[i]);
		//if (strncmp(argv[i], "--no-exit", 9) == 0) no_exit = 1;
		if (sargv == "--no-exit") std::cout << "Hello" << std::endl;
	}

	drop_stderr();
	//std::cout << "World" << std::endl;

	int ta_argc = 1;
	char *ta_argv[] = {const_cast<char *>("App")};
	gApplication = new TApplication("App", &ta_argc, ta_argv);
	g_mtx_disp = new TMutex();

	// Popup the GUI...
	af = new AppFrame(gClient->GetRoot(), 200, 200);
	c1 = af->GetCanvas();
	c1->cd(0)->SetGrid();
	c1->Update();

#if 0
        c1->SetFillColor(39);

	l->SetLineColor(0);
	l->SetLineWidth(5);
#endif

	TThread threader("Reader", &reader, NULL);
	threader.Run();
	//TThread thupdater("Updater", &updater, NULL);
	//thupdater.Run();

	//gApplication->Run();

	while (true) {
		g_mtx_disp->Lock();
		gSystem->ProcessEvents();
		g_mtx_disp->UnLock();
		//TThread::Sleep(0, 100000);
		TThread::Sleep(0, 200000);
	}
	//threader.Join();
	//thupdater.Join();

	return 0;
}
