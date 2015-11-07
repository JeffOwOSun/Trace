//
// TraceUI.h
//
// Handles FLTK integration and other user interface tasks
//
#include <cstdio>
#include <ctime>
#include <cstring>
#include <chrono>
#include <future>
#include <vector>

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

void TraceUI::cb_load_background_image(Fl_Menu_* o, void* v)
{
	TraceUI* pUI = whoami(o);

	char* newfile = fl_file_chooser("Open Image?", "*.bmp", NULL);
	if (newfile != NULL) 
	{
		pUI->raytracer->loadBackground(newfile);
	}
}

void TraceUI::cb_clear_background_image(Fl_Menu_* o, void* v)
{
	TraceUI* pUI = whoami(o);
	pUI->raytracer->clearBackground();
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

void TraceUI::cb_softShadowButton(Fl_Widget *o, void*)
{
	((TraceUI*)(o->user_data()))->m_is_enable_soft_shadow ^= true;
}

void TraceUI::cb_fresnelSwitch(Fl_Widget *o, void*)
{
	((TraceUI*)(o->user_data()))->m_is_enable_fresnel ^= true;
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

void TraceUI::cb_threadSlides(Fl_Widget* o, void*)
{
	((TraceUI*)(o->user_data()))->m_thread = ((Fl_Slider*)o)->value();
}

void TraceUI::RenderWorker(TraceUI *ui, const int from_y, const int to_y, const int w)
{
	for (int y = from_y; y < to_y && !done; ++y)
	{
		for (int x = 0; x < w && !done; ++x)
		{
			ui->raytracer->tracePixel(x, y);
		}
	}
}


void TraceUI::cb_render(Fl_Widget* o, void* v)
{
	char buffer[256];

	TraceUI* pUI = ((TraceUI*)(o->user_data()));

	if (pUI->raytracer->sceneLoaded()) {

		int width = pUI->getSize();
		int	height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
		pUI->m_traceGlWindow->resizeWindow(width, height);

		pUI->m_traceGlWindow->show();

		pUI->raytracer->traceSetup(width, height);

		// Save the window label
		const char *old_label = pUI->m_traceGlWindow->label();

		// start to render here
		done = false;
		clock_t prev, now;
		prev = clock();

		pUI->m_traceGlWindow->refresh();
		Fl::check();
		Fl::flush();

		vector<future<void>> workers;
		const int partition = height / pUI->getThread();
		for (int i = 0; i < pUI->getThread() - 1; ++i)
		{
			workers.push_back(async(launch::async, RenderWorker, pUI,
				partition * i, partition * (i + 1), width));
		}
		workers.push_back(async(launch::async, RenderWorker, pUI,
			partition * (pUI->getThread() - 1), height, width));

		bool is_all_joined = false;
		do
		{
			// current time
			now = clock();

			// check event every 1/2 second
			if (((double)(now - prev) / CLOCKS_PER_SEC)>0.5)
			{
				prev = now;

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

			is_all_joined = true;
			for (const auto &w : workers)
			{
				if (w.wait_for(chrono::milliseconds(100))
					!= future_status::ready)
				{
					is_all_joined = false;
				}
			}
		} while (!is_all_joined);

		done = true;
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
		{ "&Save Image...",	FL_ALT + 's', (Fl_Callback *)TraceUI::cb_save_image },
		{ "&Load Background...", FL_ALT + 'b', (Fl_Callback *)TraceUI::cb_load_background_image },
		{ "&Clear Background...", FL_ALT + 'c', (Fl_Callback *)TraceUI::cb_clear_background_image },
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
	m_is_enable_soft_shadow = false;
	m_is_enable_fresnel = false;
	m_thread = 1;
	m_mainWindow = new Fl_Window(100, 40, 380, 200, "Ray <Not Loaded>");
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

		m_threadSlider = new Fl_Value_Slider(10, 80, 180, 20, "Thread");
		m_threadSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_threadSlider->type(FL_HOR_NICE_SLIDER);
		m_threadSlider->labelfont(FL_COURIER);
		m_threadSlider->labelsize(12);
		m_threadSlider->minimum(1);
		m_threadSlider->maximum(8);
		m_threadSlider->step(1);
		m_threadSlider->value(m_thread);
		m_threadSlider->align(FL_ALIGN_RIGHT);
		m_threadSlider->callback(cb_threadSlides);

		m_softShadowButton = new Fl_Light_Button(240, 84, 110, 25, "Soft Shadow");
		m_softShadowButton->user_data((void*)(this));
		m_softShadowButton->value(0);
		m_softShadowButton->callback(cb_softShadowButton);

		m_fresnelSwitch = new Fl_Light_Button(240, 113, 70, 25, "Fresnel");
		m_fresnelSwitch->user_data((void*)(this));
		m_fresnelSwitch->value(0);
		m_fresnelSwitch->callback(cb_fresnelSwitch);

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