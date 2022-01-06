// example.C

#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>

#include <TQObject.h>
#include <RQ_OBJECT.h>

#include <TApplication.h>

class AppFrame {
	RQ_OBJECT("AppFrame")
private:
	TGMainFrame         *fMain;
	TRootEmbeddedCanvas *fEcanvas;
	bool m_run_state = false;
	int m_scale = 512;
	bool m_enable_ch[8] = {
		false, false, false, false,
		false, false, false, false};
public:
	AppFrame(const TGWindow *p,UInt_t w,UInt_t h);
	virtual ~AppFrame();
	void DoDraw();
	bool ToggleRunState();
	bool GetRunState();
	int ToggleScale();
	int GetScale();
	bool *DoEnableCh();
	bool *GetEnableCh();
	void PrintCanvas();
	void Exit();
	TCanvas *GetCanvas();

	ClassDef(AppFrame, 1);
};
