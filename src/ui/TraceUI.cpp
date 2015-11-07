//
// TraceUI.h
//
// Handles FLTK integration and other user interface tasks
//
#include <stdio.h>
#include <time.h>
#include <string.h>

#include <FL/fl_ask.h>

#include "TraceUI.h"
#include "../RayTracer.h"

static bool done;

//------------------------------------- Help Functions --------------------------------------------
TraceUI* TraceUI::whoami(Fl_Menu_* o)	// from menu item back to UI itself
{
	return ( (TraceUI*)(o->parent()->user_data()) );
}

//--------------------------------- Callback Functions --------------------------------------------
void TraceUI::cb_load_scene(Fl_Menu_* o, void* v) 
{
	TraceUI* pUI=whoami(o);
	
	char* newfile = fl_file_chooser("Open Scene?", "*.ray", NULL );

	if (newfile != NULL) {
		char buf[256];

		if (pUI->raytracer->loadScene(newfile)) {
			sprintf(buf, "Ray <%s>", newfile);
			done=true;	// terminate the previous rendering
		} else{
			sprintf(buf, "Ray <Not Loaded>");
		}

		pUI->m_mainWindow->label(buf);
	}
}

void TraceUI::cb_save_image(Fl_Menu_* o, void* v) 
{
	TraceUI* pUI=whoami(o);
	
	char* savefile = fl_file_chooser("Save Image?", "*.bmp", "save.bmp" );
	if (savefile != NULL) {
		pUI->m_traceGlWindow->saveImage(savefile);
	}
}

void TraceUI::cb_load_height_map(Fl_Menu_* o, void* v)
{
	TraceUI* pUI = whoami(o);

	char* newfile = fl_file_chooser("Open Height Map?", "*.bmp", NULL);

	if (newfile != NULL) {
		char buf[256];

		if (pUI->raytracer->loadHeightMap(newfile)) {
			sprintf(buf, "Height Map <%s>", newfile);
			done = true;	// terminate the previous rendering
		}
		else{
			sprintf(buf, "Height Map <Not Loaded>");
		}

		pUI->m_mainWindow->label(buf);
	}
}

void TraceUI::cb_exit(Fl_Menu_* o, void* v)
{
	TraceUI* pUI=whoami(o);

	// terminate the rendering
	done=true;

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
}

void TraceUI::cb_exit2(Fl_Widget* o, void* v) 
{
	TraceUI* pUI=(TraceUI *)(o->user_data());
	
	// terminate the rendering
	done=true;

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
}

void TraceUI::cb_about(Fl_Menu_* o, void* v) 
{
	fl_message("RayTracer Project, FLTK version for CS 341 Spring 2002. Latest modifications by Jeff Maurer, jmaurer@cs.washington.edu");
}

void TraceUI::cb_sizeSlides(Fl_Widget* o, void* v)
{
	TraceUI* pUI=(TraceUI*)(o->user_data());
	
	pUI->m_nSize=int( ((Fl_Slider *)o)->value() ) ;
	int	height = (int)(pUI->m_nSize / pUI->raytracer->aspectRatio() + 0.5);
	pUI->m_traceGlWindow->resizeWindow( pUI->m_nSize, height );
}

void TraceUI::cb_depthSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nDepth=int( ((Fl_Slider *)o)->value() ) ;
}

void TraceUI::cb_traceToggle(Fl_Widget* o, void* v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());
	pUI->m_bTrace = bool(((Fl_Light_Button*)o)->value());
}

void TraceUI::cb_causticToggle(Fl_Widget* o, void* v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());
	pUI->m_bCaustic = bool(((Fl_Light_Button*)o)->value());
}

void TraceUI::cb_photonNumberSlides(Fl_Widget* o, void* v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());

	pUI->m_nPhotonNumOrder = int(((Fl_Slider *)o)->value());
}

void TraceUI::cb_queryNumSlides(Fl_Widget* o, void* v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());

	pUI->m_nQueryNum = int(((Fl_Slider *)o)->value());
}

void TraceUI::cb_coneFilterSlides(Fl_Widget* o, void* v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());

	pUI->m_dConeAtten = int(((Fl_Slider *)o)->value());
}

void TraceUI::cb_causticAmplifySlides(Fl_Widget* o, void* v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());

	pUI->m_dCausticAmplify = int(((Fl_Slider *)o)->value());
}

void TraceUI::cb_render(Fl_Widget* o, void* v)
{
	char buffer[256];

	TraceUI* pUI=((TraceUI*)(o->user_data()));
	
	if (pUI->raytracer->sceneLoaded()) {
		int width=pUI->getSize();
		int	height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
		pUI->m_traceGlWindow->resizeWindow( width, height );

		pUI->m_traceGlWindow->show();

		pUI->raytracer->traceSetup(width, height, pUI->m_bTrace, pUI->m_bCaustic, pUI->m_nPhotonNumOrder, pUI->m_nQueryNum, pUI->m_dConeAtten, pUI->m_dCausticAmplify);
		
		// Save the window label
		const char *old_label = pUI->m_traceGlWindow->label();

		// render the photon map

		// start to render here	
		done=false;
		clock_t prev, now;
		prev=clock();
		
		pUI->m_traceGlWindow->refresh();
		Fl::check();
		Fl::flush();

		for (int y=0; y<height; y++) {
			for (int x=0; x<width; x++) {
				if (done) break;
				
				// current time
				now = clock();

				// check event every 1/2 second
				if (((double)(now-prev)/CLOCKS_PER_SEC)>0.5) {
					prev=now;

					if (Fl::ready()) {
						// refresh
						pUI->m_traceGlWindow->refresh();
						// check event
						Fl::check();

						if (Fl::damage()) {
							Fl::flush();
						}
					}
				}

				pUI->raytracer->tracePixel( x, y );
		
			}
			if (done) break;

			// flush when finish a row
			if (Fl::ready()) {
				// refresh
				pUI->m_traceGlWindow->refresh();

				if (Fl::damage()) {
					Fl::flush();
				}
			}
			// update the window label
			sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)height * 100.0), old_label);
			pUI->m_traceGlWindow->label(buffer);
			
		}
		done=true;
		pUI->m_traceGlWindow->refresh();

		// Restore the window label
		pUI->m_traceGlWindow->label(old_label);		
	}
}

void TraceUI::cb_stop(Fl_Widget* o, void* v)
{
	done=true;
}

void TraceUI::show()
{
	m_mainWindow->show();
}

void TraceUI::setRayTracer(RayTracer *tracer)
{
	raytracer = tracer;
	m_traceGlWindow->setRayTracer(tracer);
}

int TraceUI::getSize()
{
	return m_nSize;
}

int TraceUI::getDepth()
{
	return m_nDepth;
}

// menu definition
Fl_Menu_Item TraceUI::menuitems[] = {
	{ "&File",		0, 0, 0, FL_SUBMENU },
		{ "&Load Scene...",	FL_ALT + 'l', (Fl_Callback *)TraceUI::cb_load_scene },
		{ "&Save Image...", FL_ALT + 's', (Fl_Callback *)TraceUI::cb_save_image },
		{ "&Load Height Map...", FL_ALT + 's', (Fl_Callback *)TraceUI::cb_load_height_map },
		{ "&Exit",			FL_ALT + 'e', (Fl_Callback *)TraceUI::cb_exit },
		{ 0 },

	{ "&Help",		0, 0, 0, FL_SUBMENU },
		{ "&About",	FL_ALT + 'a', (Fl_Callback *)TraceUI::cb_about },
		{ 0 },

	{ 0 }
};

TraceUI::TraceUI() {
	// init.
	m_nDepth = 0;
	m_nSize = 150;
	m_bTrace = true;
	m_bCaustic = false;
	m_nPhotonNumOrder = 5;
	m_nQueryNum = 3;
	m_dConeAtten = -100;
	m_dCausticAmplify = 1.0;
	m_mainWindow = new Fl_Window(100, 40, 320, 215, "Ray <Not Loaded>");
		m_mainWindow->user_data((void*)(this));	// record self to be used by static callback functions
		// install menu bar
		m_menubar = new Fl_Menu_Bar(0, 0, 320, 25);
		m_menubar->menu(menuitems);

		// install slider depth
		m_depthSlider = new Fl_Value_Slider(10, 30, 180, 20, "Depth");
		m_depthSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_depthSlider->type(FL_HOR_NICE_SLIDER);
        m_depthSlider->labelfont(FL_COURIER);
        m_depthSlider->labelsize(12);
		m_depthSlider->minimum(0);
		m_depthSlider->maximum(10);
		m_depthSlider->step(1);
		m_depthSlider->value(m_nDepth);
		m_depthSlider->align(FL_ALIGN_RIGHT);
		m_depthSlider->callback(cb_depthSlides);

		// install slider size
		m_sizeSlider = new Fl_Value_Slider(10, 55, 180, 20, "Size");
		m_sizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_sizeSlider->type(FL_HOR_NICE_SLIDER);
        m_sizeSlider->labelfont(FL_COURIER);
        m_sizeSlider->labelsize(12);
		m_sizeSlider->minimum(64);
		m_sizeSlider->maximum(512);
		m_sizeSlider->step(1);
		m_sizeSlider->value(m_nSize);
		m_sizeSlider->align(FL_ALIGN_RIGHT);
		m_sizeSlider->callback(cb_sizeSlides);

		// install button for caustic rendering
		m_traceButton = new Fl_Light_Button(10, 80, 90, 20, "Trace");
		m_traceButton->user_data((void*)(this));	// record self to be used by static callback functions
		m_traceButton->value(m_bTrace);//default value
		m_traceButton->callback(cb_traceToggle);

		m_causticButton = new Fl_Light_Button(110, 80, 90, 20, "Caustic");
		m_causticButton->user_data((void*)(this));	// record self to be used by static callback functions
		m_causticButton->callback(cb_causticToggle);

		// install slider photon number
		m_photonNumSlider = new Fl_Value_Slider(10, 105, 180, 20, "Photon Num 10^n");
		m_photonNumSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_photonNumSlider->type(FL_HOR_NICE_SLIDER);
		m_photonNumSlider->labelfont(FL_COURIER);
		m_photonNumSlider->labelsize(12);
		m_photonNumSlider->minimum(1);
		m_photonNumSlider->maximum(8);
		m_photonNumSlider->step(1);
		m_photonNumSlider->value(m_nPhotonNumOrder);
		m_photonNumSlider->align(FL_ALIGN_RIGHT);
		m_photonNumSlider->callback(cb_photonNumberSlides);

		// install slider query number
		m_queryNumSlider = new Fl_Value_Slider(10, 130, 180, 20, "Query Num");
		m_queryNumSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_queryNumSlider->type(FL_HOR_NICE_SLIDER);
		m_queryNumSlider->labelfont(FL_COURIER);
		m_queryNumSlider->labelsize(12);
		m_queryNumSlider->minimum(1);
		m_queryNumSlider->maximum(100);
		m_queryNumSlider->step(1);
		m_queryNumSlider->value(m_nQueryNum);
		m_queryNumSlider->align(FL_ALIGN_RIGHT);
		m_queryNumSlider->callback(cb_queryNumSlides);

		// install slider cone filter
		m_coneFilterSlider = new Fl_Value_Slider(10, 155, 180, 20, "Cone Filter");
		m_coneFilterSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_coneFilterSlider->type(FL_HOR_NICE_SLIDER);
		m_coneFilterSlider->labelfont(FL_COURIER);
		m_coneFilterSlider->labelsize(12);
		m_coneFilterSlider->minimum(-10000);
		m_coneFilterSlider->maximum(0);
		m_coneFilterSlider->step(1);
		m_coneFilterSlider->value(m_dConeAtten);
		m_coneFilterSlider->align(FL_ALIGN_RIGHT);
		m_coneFilterSlider->callback(cb_coneFilterSlides);

		// install slider caustic amplification
		m_causticAmplifySlider = new Fl_Value_Slider(10, 180, 180, 20, "Caustic Amplify");
		m_causticAmplifySlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_causticAmplifySlider->type(FL_HOR_NICE_SLIDER);
		m_causticAmplifySlider->labelfont(FL_COURIER);
		m_causticAmplifySlider->labelsize(12);
		m_causticAmplifySlider->minimum(1);
		m_causticAmplifySlider->maximum(255);
		m_causticAmplifySlider->step(0.1);
		m_causticAmplifySlider->value(m_dCausticAmplify);
		m_causticAmplifySlider->align(FL_ALIGN_RIGHT);
		m_causticAmplifySlider->callback(cb_causticAmplifySlides);

		m_renderButton = new Fl_Button(240, 27, 70, 25, "&Render");
		m_renderButton->user_data((void*)(this));
		m_renderButton->callback(cb_render);

		m_stopButton = new Fl_Button(240, 55, 70, 25, "&Stop");
		m_stopButton->user_data((void*)(this));
		m_stopButton->callback(cb_stop);

		m_mainWindow->callback(cb_exit2);
		m_mainWindow->when(FL_HIDE);
    m_mainWindow->end();

	// image view
	m_traceGlWindow = new TraceGLWindow(100, 150, m_nSize, m_nSize, "Rendered Image");
	m_traceGlWindow->end();
	m_traceGlWindow->resizable(m_traceGlWindow);
}