// example.C

#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <RQ_OBJECT.h>
#include <TROOT.h>

#include <TApplication.h>
#include <TMutex.h>

#include "appframe.h"

extern TMutex *g_mtx_disp;

AppFrame::AppFrame(const TGWindow *p, UInt_t w, UInt_t h) {
	// Create a main frame
	fMain = new TGMainFrame(p, w, h);

	TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain, 480, 360);
	fMain->AddFrame(hframe,
		new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));

	// Create canvas widget
	fEcanvas = new TRootEmbeddedCanvas("Ecanvas", hframe, 480, 360);
	hframe->AddFrame(fEcanvas,
		new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 4, 4, 4, 4));

	// Create a horizontal frame widget with buttons
	TGVerticalFrame *vframe = new TGVerticalFrame(hframe, 60, 300);

	TGTextButton *bdraw = new TGTextButton(vframe, "&Run/Stop");
	//bdraw->Connect("Clicked()", "AppFrame", this, "DoDraw()");
	bdraw->Connect("Clicked()", "AppFrame", this, "ToggleRunState()");
	vframe->AddFrame(bdraw,
		new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 2, 4, 4, 4));

	TGTextButton *btoggle = new TGTextButton(vframe, "&Y-Scale");
	btoggle->Connect("Clicked()", "AppFrame", this, "ToggleScale()");
	vframe->AddFrame(btoggle,
		new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 2, 4, 4, 4));

	TGTextButton *bxscale = new TGTextButton(vframe, "&X-scale");
	bxscale->Connect("Clicked()", "AppFrame", this, "ToggleXScale()");
	vframe->AddFrame(bxscale,
		new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 2, 4, 4, 4));

	TGCheckButton* bcheck[8];
	for (int i = 0 ; i < 8 ; i++) {
		char title[8];
		title[0] = '0' + i;
		title[1] = '\0';
		bcheck[i] = new TGCheckButton(vframe, title, 70 + i);
		vframe->AddFrame(bcheck[i],
			new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 2, 4, 4, 4));
		bcheck[i]->Connect("Toggled(Bool_t)", "AppFrame", this, "DoEnableCh()");
	}

	TGTextButton *bexit = new TGTextButton(vframe,"&Exit",
		"gApplication->Terminate(0)");
	vframe->AddFrame(bexit,
		new TGLayoutHints(
			//kLHintsCenterX | kLHintsExpandX | kLHintsBottom,
			kLHintsExpandX | kLHintsBottom,
			2, 4, 4, 4));

	TGTextButton *bprint = new TGTextButton(vframe,"&Print");
	bprint->Connect("Clicked()", "AppFrame", this, "PrintCanvas()");
	vframe->AddFrame(bprint,
		new TGLayoutHints(
			//kLHintsCenterX | kLHintsExpandX | kLHintsBottom,
			kLHintsExpandX | kLHintsBottom,
			2, 4, 4, 4));


	hframe->AddFrame(vframe,
		new TGLayoutHints(kLHintsRight | kLHintsExpandY, 0, 0, 0, 0));

	// Set a name to the main frame
	hframe->SetWindowName("App Frame");

	// Map all subwindows of main frame
	fMain->MapSubwindows();

	// Initialize the layout algorithm
	fMain->Resize(fMain->GetDefaultSize());

	// Map main frame
	fMain->MapWindow();
}

AppFrame::~AppFrame() {
	// Clean up used widgets: frames, buttons, layout hints
	fMain->Cleanup();
	delete fMain;
}

TCanvas* AppFrame::GetCanvas() {
	return fEcanvas->GetCanvas();
}


ClassImp(AppFrame);

void AppFrame::DoDraw() {
	// Draws function graphics in randomly chosen interval
	TF1 *f1 = new TF1("f1","sin(x)/x",0,gRandom->Rndm()*10);
	f1->SetLineWidth(3);
	f1->Draw();
	TCanvas *fCanvas = fEcanvas->GetCanvas();
	fCanvas->cd();
	fCanvas->Update();
}

bool AppFrame::ToggleRunState() {
	m_run_state = (! m_run_state);
	return m_run_state;
}

bool AppFrame::GetRunState() {
	return m_run_state;
}

int AppFrame::ToggleScale() {
	const int scale_factor[] = {512, 256, 128, 64, 32, 16};
	static int sf_index = 0;

	if (++sf_index > 5) sf_index = 0;
	m_scale = scale_factor[sf_index];

	//std::cout << "#D scale " << sf_index << " " << m_scale << std::endl;

	return m_scale;
}

int AppFrame::GetScale() {
	return m_scale;
}

int AppFrame::ToggleXScale() {
	const int scale_factor[] = {512, 256, 128, 64, 32, 16};
	static int sf_index = 0;

	if (++sf_index > 5) sf_index = 0;
	m_xscale = scale_factor[sf_index];

	//std::cout << "#D xscale " << sf_index << " " << m_xscale << std::endl;

	return m_xscale;
}

int AppFrame::GetXScale() {
	return m_xscale;
}

bool* AppFrame::DoEnableCh() {

	TGButton *btn = (TGButton *) gTQSender;
	int id = btn->WidgetId();

	//std::cout << "DoButton: id = " << id << std::endl;
	int button = id - 70;
	m_enable_ch[button] = ! m_enable_ch[button];

	for (int i = 0 ; i < 8 ; i++) std::cout << " " << m_enable_ch[i]  << " ";
	for (int i = 0 ; i < 8 ; i++) {
		if (m_enable_ch[i] == true) {
			std::cout << "O";
		} else {
			std::cout << "-";
		}
	}
	std::cout << std::endl;

	return m_enable_ch;
}

bool* AppFrame::GetEnableCh() {
	return m_enable_ch;
}

void AppFrame::PrintCanvas() {
	TCanvas *c = fEcanvas->GetCanvas();
	c->Print("display.png", "png");
	c->Print("display.pdf", "pdf");
	return;
}

void AppFrame::Exit() {
	delete g_mtx_disp;
	gApplication->Terminate(0);
	return;
}
