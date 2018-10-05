#include <algorithm>
#ifdef USE_SDL2
#include <guisan.hpp>
#include <SDL_ttf.h>
#include <guisan/sdl.hpp>
#include <guisan/sdl/sdltruetypefont.hpp>
#else
#include <guichan.hpp>
#include <SDL/SDL_ttf.h>
#include <guichan/sdl.hpp>
#include "sdltruetypefont.hpp"
#endif
#include <iostream>
#include <sstream>
#include "SelectorEntry.hpp"

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "uae.h"
#include "gui_handling.h"


#define DIALOG_WIDTH 520
#define DIALOG_HEIGHT 400

static bool dialogResult = false;
static bool dialogFinished = false;
static char workingDir[MAX_PATH];

static gcn::Window *wndSelectFolder;
static gcn::Button* cmdOK;
static gcn::Button* cmdCancel;
static gcn::ListBox* lstFolders;
static gcn::ScrollArea* scrAreaFolders;
static gcn::TextField *txtCurrent;


class ButtonActionListener : public gcn::ActionListener
{
  public:
    void action(const gcn::ActionEvent& actionEvent)
    {
      if (actionEvent.getSource() == cmdOK)
      {
        dialogResult = true;
      }
      dialogFinished = true;
    }
};
static ButtonActionListener* buttonActionListener;


class DirListModel : public gcn::ListModel
{
  std::vector<std::string> dirs;

  public:
    DirListModel(const char * path)
    {
      changeDir(path);
    }
      
    int getNumberOfElements()
    {
      return dirs.size();
    }
      
    std::string getElementAt(int i)
    {
      if(i >= dirs.size() || i < 0)
        return "---";
      return dirs[i];
    }
      
    void changeDir(const char *path)
    {
      ReadDirectory(path, &dirs, NULL);
      if(dirs.size() == 0)
        dirs.push_back("..");
    }
};
static DirListModel dirList(".");


static void checkfoldername (char *current)
{
	char *ptr;
	char actualpath [MAX_PATH];
	DIR *dir;
	
	if (dir = opendir(current))
	{ 
	  dirList = current;
	  ptr = realpath(current, actualpath);
	  strncpy(workingDir, ptr, MAX_PATH - 1);
	  closedir(dir);
	}
  else
    strncpy(workingDir, start_path_data, MAX_PATH - 1);
  txtCurrent->setText(workingDir);
}


class ListBoxActionListener : public gcn::ActionListener
{
  public:
    void action(const gcn::ActionEvent& actionEvent)
    {
      int selected_item;
      char foldername[MAX_PATH] = "";

      selected_item = lstFolders->getSelected();
      strncpy(foldername, workingDir, MAX_PATH - 1);
      strncat(foldername, "/", MAX_PATH - 1);
      strncat(foldername, dirList.getElementAt(selected_item).c_str(), MAX_PATH - 1);
      checkfoldername(foldername);
    }
};
static ListBoxActionListener* listBoxActionListener;


static void InitSelectFolder(const char *title)
{
	wndSelectFolder = new gcn::Window("Load");
	wndSelectFolder->setSize(DIALOG_WIDTH, DIALOG_HEIGHT);
  wndSelectFolder->setPosition((GUI_WIDTH - DIALOG_WIDTH) / 2, (GUI_HEIGHT - DIALOG_HEIGHT) / 2);
  wndSelectFolder->setBaseColor(gui_baseCol + 0x202020);
  wndSelectFolder->setCaption(title);
  wndSelectFolder->setTitleBarHeight(TITLEBAR_HEIGHT);
  
  buttonActionListener = new ButtonActionListener();
  
	cmdOK = new gcn::Button("Ok");
	cmdOK->setSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	cmdOK->setPosition(DIALOG_WIDTH - DISTANCE_BORDER - 2 * BUTTON_WIDTH - DISTANCE_NEXT_X, DIALOG_HEIGHT - 2 * DISTANCE_BORDER - BUTTON_HEIGHT - 10);
  cmdOK->setBaseColor(gui_baseCol + 0x202020);
  cmdOK->addActionListener(buttonActionListener);
  
	cmdCancel = new gcn::Button("Cancel");
	cmdCancel->setSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	cmdCancel->setPosition(DIALOG_WIDTH - DISTANCE_BORDER - BUTTON_WIDTH, DIALOG_HEIGHT - 2 * DISTANCE_BORDER - BUTTON_HEIGHT - 10);
  cmdCancel->setBaseColor(gui_baseCol + 0x202020);
  cmdCancel->addActionListener(buttonActionListener);

  txtCurrent = new gcn::TextField();
  txtCurrent->setSize(DIALOG_WIDTH - 2 * DISTANCE_BORDER - 4, TEXTFIELD_HEIGHT);
  txtCurrent->setPosition(DISTANCE_BORDER, 10);
  txtCurrent->setEnabled(false);

  listBoxActionListener = new ListBoxActionListener();
  
  lstFolders = new gcn::ListBox(&dirList);
  lstFolders->setSize(800, 252);
  lstFolders->setBaseColor(gui_baseCol);
  lstFolders->setWrappingEnabled(true);
  lstFolders->addActionListener(listBoxActionListener);
  
  scrAreaFolders = new gcn::ScrollArea(lstFolders);
#ifdef USE_SDL2
	scrAreaFolders->setBorderSize(1);
#else
  scrAreaFolders->setFrameSize(1);
#endif
  scrAreaFolders->setPosition(DISTANCE_BORDER, 10 + TEXTFIELD_HEIGHT + 10);
  scrAreaFolders->setSize(DIALOG_WIDTH - 2 * DISTANCE_BORDER - 4, 272);
  scrAreaFolders->setScrollbarWidth(20);
  scrAreaFolders->setBaseColor(gui_baseCol + 0x202020);
  
  wndSelectFolder->add(cmdOK);
  wndSelectFolder->add(cmdCancel);
  wndSelectFolder->add(txtCurrent);
  wndSelectFolder->add(scrAreaFolders);
  
  gui_top->add(wndSelectFolder);
  
  lstFolders->requestFocus();
  lstFolders->setSelected(0);
  wndSelectFolder->requestModalFocus();
}


static void ExitSelectFolder(void)
{
  wndSelectFolder->releaseModalFocus();
  gui_top->remove(wndSelectFolder);

  delete cmdOK;
  delete cmdCancel;
  delete buttonActionListener;
  
  delete txtCurrent;
  delete lstFolders;
  delete scrAreaFolders;
  delete listBoxActionListener;
  
  delete wndSelectFolder;
}


static void SelectFolderLoop(void)
{
#ifndef USE_SDL2
  FocusBugWorkaround(wndSelectFolder);
#endif

  while(!dialogFinished)
  {
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
      if (event.type == SDL_KEYDOWN)
      {
        switch(event.key.keysym.sym)
        {
          case VK_ESCAPE:
            dialogFinished = true;
            break;
            
          case VK_LEFT:
            {
              gcn::FocusHandler* focusHdl = gui_top->_getFocusHandler();
              gcn::Widget* activeWidget = focusHdl->getFocused();
              if(activeWidget == lstFolders)
                cmdCancel->requestFocus();
              else if(activeWidget == cmdCancel)
                cmdOK->requestFocus();
              else if(activeWidget == cmdOK)
                lstFolders->requestFocus();
              continue;
            }
            break;
            
          case VK_RIGHT:
            {
              gcn::FocusHandler* focusHdl = gui_top->_getFocusHandler();
              gcn::Widget* activeWidget = focusHdl->getFocused();
              if(activeWidget == lstFolders)
                cmdOK->requestFocus();
              else if(activeWidget == cmdCancel)
                lstFolders->requestFocus();
              else if(activeWidget == cmdOK)
                cmdCancel->requestFocus();
              continue;
            }
            break;

				  case VK_X:
				  case VK_A:
            event.key.keysym.sym = SDLK_RETURN;
            gui_input->pushInput(event); // Fire key down
            event.type = SDL_KEYUP;  // and the key up
            break;
				  default: 
					  break;
        }
      }

      //-------------------------------------------------
      // Send event to guichan/guisan-controls
      //-------------------------------------------------
      gui_input->pushInput(event);
    }

    // Now we let the Gui object perform its logic.
    uae_gui->logic();
    // Now we let the Gui object draw itself.
    uae_gui->draw();
    // Finally we update the screen.
    wait_for_vsync();
#ifdef USE_SDL2
		UpdateGuiScreen();
#else
    SDL_Flip(gui_screen);
#endif
  }  
}


bool SelectFolder(const char *title, char *value)
{
  dialogResult = false;
  dialogFinished = false;
  InitSelectFolder(title);
  checkfoldername(value);
  SelectFolderLoop();
  ExitSelectFolder();
  if(dialogResult)
  {
    strncpy(value, workingDir, MAX_PATH - 1);
    if(value[strlen(value) - 1] != '/')
      strncat(value, "/", MAX_PATH - 1);
  }
  return dialogResult;
}
