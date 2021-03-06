//----------------------------------------------------------------------------
// File: PCMPlay.cpp
//
// Author- Parag Havaldar
// Desc: The PCMPlay sample shows how to load and play a PCM file using
//       a DirectSound buffer.
//
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <Commctrl.h>
#include "basetsd.h"
#include <commdlg.h>
#include <mmreg.h>
#include <dxerr.h>
#include <dsound.h>
#include "resource.h"
#include "CVideo.h"
#include "Image.h"
#include "CS576SoundUtil.h"
#include "DXUtil.h"

#define THUMBNAIL_X_OFFSET 34

//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
VOID    OnInitDialog( HWND hDlg );
VOID    OnOpenSoundFile( HWND hDlg );
HRESULT OnPlaySound( HWND hDlg );
VOID    OnTimer( HWND hDlg );
VOID    EnablePlayUI( HWND hDlg, VIDEO_STATE_E _eState);
VOID	DrawThumbnails(HWND, int);
VOID    DrawSetFrame(HWND, int);

//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
CSoundManager* g_pSoundManager = NULL;
CSound*        g_pSound = NULL;
BOOL           g_bBufferPaused;
CVideo*        g_pMyVideo;
MyImage        g_outImage;
vector <unsigned short> g_IFrames;
vector <unsigned short> g_ASFrames;
int g_w, g_h;
int g_nCurrentScrollbarPos;

char FramePath[_MAX_PATH];
char AudioPath[_MAX_PATH];
BITMAPINFO g_bmi;

vector <unsigned short> g_summarizationFrames;

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
	g_w = 480; g_h = 270;

	sscanf(pCmdLine, "%s %s", &FramePath, &AudioPath);

    // Display the main dialog box.
	if (strstr(FramePath, ".rgb") == NULL)
	{
		MessageBox(NULL, "1:Incorrect input file format. "
			"Sample will now exit.", "AV Player Sample",
			MB_OK | MB_ICONERROR);
		return FALSE;
	}
	else
	{
		g_pMyVideo = new CVideo();
		g_pMyVideo->setVideoPath(FramePath);

		if (g_pMyVideo == NULL)
		{
			MessageBox(NULL, "Incorrect input file format. "
				"Sample will now exit.", "AV Player Sample",
				MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}
	DialogBox(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);

    return TRUE;
}//WinMain

//-----------------------------------------------------------------------------
// Name: MainDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hdc;
	unsigned short unMin, unSec, unSubSec;
	char str[64] = { 0 };
	static clock_t iterationTime;

    switch( msg ) 
    {
        case WM_INITDIALOG:
            OnInitDialog( hDlg );
			OnOpenSoundFile(hDlg);
			g_pMyVideo->createVideo(g_w, g_h);
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                //case IDC_SOUNDFILE:
                //    OnOpenSoundFile( hDlg );
                //    break;

                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    break;

                case IDC_PLAY:
				{
					BOOL checked = IsDlgButtonChecked(hDlg, IDC_CORRECT_CHECK);

					if (g_pMyVideo->getVideoState() == VIDEO_STATE_PLAYING)
					{
						g_pMyVideo->pauseVideo();
					}
					else
					{
						if (checked)
						{
							char pCorrectedFilePath[128] = { 0 };
							char* addr;
							sprintf(pCorrectedFilePath, "%s", FramePath);
							addr = strchr(pCorrectedFilePath, 'r');
							sprintf(--addr, "%s", "C.rgb");
							g_pMyVideo->setVideoPath(pCorrectedFilePath);
						}

						g_pMyVideo->playVideo((bool)checked);
					}

					int noFrames = g_pMyVideo->getNoFrames();
					unMin = floor((noFrames / FRAME_RATE_HZ) / 60);
					unSec = floor((noFrames / FRAME_RATE_HZ) % 60);
					unSubSec = (noFrames % FRAME_RATE_HZ);

					sprintf(str, "%02d:%02d.%02d", unMin, unSec, unSubSec);
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_END), str);
				}//IDC_PLAY
				break;
                case IDC_STOP:
                    if( g_pSound )
                    {
                        g_pSound->Stop();
                        g_pSound->Reset();
                    }

					if (g_pMyVideo)
					{
						g_pMyVideo->stopVideo();
					}

                    EnablePlayUI( hDlg, VIDEO_STATE_STOPPED);
                    break;
				case IDC_ANALYZE:
				{
					EnablePlayUI(hDlg, VIDEO_STATE_ANALYZING);
					if (g_pMyVideo)
					{
						SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, g_pMyVideo->getNoFrames()));
						SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBS_SMOOTH, 0, 1);
						SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETSTEP, (WPARAM)1, 0);
						g_pMyVideo->analyzeVideo();
						//g_ASFrames = g_pMyVideo->getSyncFrames(".\\video\\Alin_Day1_002 - Copy.wav");

						int noFrames = g_pMyVideo->getNoFrames();
						unMin = floor((noFrames / FRAME_RATE_HZ) / 60);
						unSec = floor((noFrames / FRAME_RATE_HZ) % 60);
						unSubSec = (noFrames % FRAME_RATE_HZ);

						sprintf(str, "%02d:%02d.%02d", unMin, unSec, unSubSec);
						SetWindowText(GetDlgItem(hDlg, IDC_STATIC_END), str);
					}
				}
				break;
				case IDC_BUTTON_BROWSE:
				{
					OPENFILENAME fileName;
					char szFile[256] = { 0 };

					ZeroMemory(&fileName, sizeof(fileName));
					fileName.lStructSize = sizeof(fileName);
					fileName.hwndOwner = NULL;
					fileName.lpstrFile = szFile;
					fileName.lpstrFile[0] = '\0';
					fileName.nMaxFile = sizeof(szFile);
					fileName.lpstrFilter = "RGB\0*.rgb\0";// "All\0*.*\0RGB\0*.rgb\0PNG\0*.png\0";
					fileName.nFilterIndex = 1;
					fileName.lpstrFileTitle = NULL;
					fileName.nMaxFileTitle = 0;
					fileName.lpstrInitialDir = NULL;
					fileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					GetOpenFileName(&fileName);

					if (strlen(fileName.lpstrFile))
					{
						FILE* pFile = fopen(fileName.lpstrFile, "rb");
						MyImage image;
						image.setHeight(720);
						image.setWidth(1280);
						image.ReadImage(pFile, 0);//just one frame, read first

						Mat	showImage(720, 1280, CV_8UC3, image.getImageData());

						//imshow("1", showImage);
						//waitKey(0);

						int index = g_pMyVideo->videoIndex(image);
						DrawSetFrame(hDlg, index);
						fclose(pFile);
					}
				}//IDC_BUTTON_BROWSE
				break;
                default:
                    return FALSE; // Didn't handle message
            }
            break;

        case WM_TIMER:
			sprintf(str, "%d ms\n", clock() - iterationTime);
			//OutputDebugString(_T(str));
			iterationTime = clock();
            OnTimer( hDlg );
            break;

        case WM_DESTROY:
            // Cleanup everything
            KillTimer( hDlg, 1 );    
            SAFE_DELETE(g_pSound);
            SAFE_DELETE(g_pSoundManager);
			SAFE_DELETE(g_pMyVideo);
            break; 

		case WM_PAINT:
		{
			hdc = BeginPaint(hDlg, &ps);

			memset(&g_bmi, 0, sizeof(g_bmi));
			g_bmi.bmiHeader.biSize = sizeof(g_bmi.bmiHeader);
			g_bmi.bmiHeader.biWidth = g_pMyVideo->getVideoWidth();
			g_bmi.bmiHeader.biHeight = -g_pMyVideo->getVideoHeight();  // Use negative height.  DIB is top-down.
			g_bmi.bmiHeader.biPlanes = 1;
			g_bmi.bmiHeader.biBitCount = 24;
			g_bmi.bmiHeader.biCompression = BI_RGB;
			g_bmi.bmiHeader.biSizeImage = g_pMyVideo->getVideoWidth()*g_pMyVideo->getVideoHeight();

			EndPaint(hDlg, &ps);
		}
		break;
		case WM_HSCROLL:
		{
			switch (LOWORD(wParam))
			{
				case SB_LINELEFT:
				case SB_PAGELEFT:
					g_nCurrentScrollbarPos = GetScrollPos(GetDlgItem(hDlg, IDC_SCROLLBAR1), SB_CTL);
					if (g_nCurrentScrollbarPos - 1 > 0)
						g_nCurrentScrollbarPos -= 1;
					else if (g_nCurrentScrollbarPos > 0)
						g_nCurrentScrollbarPos = 0;
					DrawThumbnails(hDlg, g_nCurrentScrollbarPos);
					SetScrollPos(GetDlgItem(hDlg, IDC_SCROLLBAR1), SB_CTL, g_nCurrentScrollbarPos, TRUE);
				break;
				case SB_LINERIGHT:
				case SB_PAGERIGHT:
					g_nCurrentScrollbarPos = GetScrollPos(GetDlgItem(hDlg, IDC_SCROLLBAR1), SB_CTL);
					if (g_nCurrentScrollbarPos + 1 < g_IFrames.size())
						g_nCurrentScrollbarPos += 1;
					else if (g_nCurrentScrollbarPos < g_IFrames.size())
						g_nCurrentScrollbarPos = g_IFrames.size();
					DrawThumbnails(hDlg, g_nCurrentScrollbarPos);
					SetScrollPos(GetDlgItem(hDlg, IDC_SCROLLBAR1), SB_CTL, g_nCurrentScrollbarPos, TRUE);
				break;
				case SB_ENDSCROLL:
					break;
			}//switch

		}//WM_HSCROLL
		break;
		case WM_LBUTTONUP:
		{
			if (g_pMyVideo)
			{
				POINT mousePos;
				GetCursorPos(&mousePos);
				ScreenToClient(hDlg, &mousePos);

				char str[32] = { 0 };
				sprintf(str, "(%d,%d)\n", mousePos.x, mousePos.y);
				OutputDebugString(_T(str));

				if (mousePos.y > 320 && mousePos.y < 320 + 270 / 4 &&
					mousePos.x > THUMBNAIL_X_OFFSET && mousePos.x < 480 + 4)
				{
					//1st
					if (mousePos.x < THUMBNAIL_X_OFFSET + 480 / 4 + 1)
					{
						if (g_IFrames.size())
						{
							int index = g_IFrames.at(g_nCurrentScrollbarPos);
							DrawSetFrame(hDlg, index);
						}
					}
					//2nd
					else if (mousePos.x < THUMBNAIL_X_OFFSET + 480 / 2 + 2)
					{
						if (g_IFrames.size())
						{
							int index = g_IFrames.at(g_nCurrentScrollbarPos+1);
							DrawSetFrame(hDlg, index);
						}
					}
					//3rd
					else if (mousePos.x < THUMBNAIL_X_OFFSET + 3 * 480 / 4 + 3)
					{
						if (g_IFrames.size())
						{
							int index = g_IFrames.at(g_nCurrentScrollbarPos + 2);
							DrawSetFrame(hDlg, index);
						}
					}
					//4th
					else
					{
						if (g_IFrames.size())
						{
							int index = g_IFrames.at(g_nCurrentScrollbarPos + 3);
							DrawSetFrame(hDlg, index);
						}
					}
				}//if
			}
		}//WM_LBUTTONDBLCLK
		break;
        default:
            return FALSE; // Didn't handle message
    }

    return TRUE; // Handled message
}//MainDlgProc

//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Initializes the dialogs (sets up UI controls, etc.)
//-----------------------------------------------------------------------------
VOID OnInitDialog( HWND hDlg )
{
    HRESULT hr;

    // Load the icon
#ifdef _WIN64
    HINSTANCE hInst = (HINSTANCE) GetWindowLongPtr( hDlg, GWLP_HINSTANCE );
#else
    HINSTANCE hInst = (HINSTANCE) GetWindowLong( hDlg, GWL_HINSTANCE );
#endif
    HICON hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDR_MAINFRAME ) );

    // Set the icon for this dialog.
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    // Create a static IDirectSound in the CSound class.  
    // Set coop level to DSSCL_PRIORITY, and set primary buffer 
    // format to stereo, 22kHz and 16-bit output.
    g_pSoundManager = new CSoundManager();

    if( FAILED( hr = g_pSoundManager->Initialize( hDlg, DSSCL_PRIORITY, 2, 22050, 16 ) ) )
    {
        DXTRACE_ERR( TEXT("Initialize"), hr );
        MessageBox( hDlg, "Error initializing DirectSound.  Sample will now exit.", 
                            "DirectSound Sample", MB_OK | MB_ICONERROR );
        EndDialog( hDlg, IDABORT );
        return;
    }

    g_bBufferPaused = FALSE;

	SetScrollRange(GetDlgItem(hDlg, IDC_SCROLLBAR1), SB_CTL, 0, 100, TRUE);

    //Create a timer, so we can check for when the soundbuffer is stopped
	//also use for Video
    SetTimer(hDlg, 0, 1000/FRAME_RATE_HZ, NULL );
}//OnInitDialog

//-----------------------------------------------------------------------------
// Name: OnOpenSoundFile()
// Desc: Called when the user requests to open a sound file
//-----------------------------------------------------------------------------
VOID OnOpenSoundFile( HWND hDlg ) 
{
    HRESULT hr;

    if( g_pSound )
    {
        g_pSound->Stop();
        g_pSound->Reset();
    }

    // Free any previous sound, and make a new one
    SAFE_DELETE( g_pSound );

    // Load the wave file into a DirectSound buffer
    if( FAILED( hr = g_pSoundManager->Create( &g_pSound, AudioPath, 0, GUID_NULL ) ) )
    {
        // Not a critical failure, so just update the status
		DXTRACE_ERR_MSGBOX( TEXT("Create"), hr );
        //SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Could not create sound buffer.") );
        return; 
    }

    // Update the UI controls to show the sound as the file is loaded
    //SetDlgItemText( hDlg, IDC_FILENAME, AudioPath);
    EnablePlayUI( hDlg, VIDEO_STATE_STOPPED);
}


//-----------------------------------------------------------------------------
// Name: OnPlaySound()
// Desc: User hit the "Play" button
//-----------------------------------------------------------------------------
HRESULT OnPlaySound( HWND hDlg ) 
{
    HRESULT hr;

    HWND hLoopButton = GetDlgItem( hDlg, IDC_LOOP_CHECK );
    BOOL bLooped = ( SendMessage( hLoopButton, BM_GETSTATE, 0, 0 ) == BST_CHECKED );

    if( g_bBufferPaused )
    {
        // Play the buffer since it is currently paused
        DWORD dwFlags = bLooped ? DSBPLAY_LOOPING : 0L;
        if( FAILED( hr = g_pSound->Play( 0, dwFlags ) ) )
            return DXTRACE_ERR( TEXT("Play"), hr );

        // Update the UI controls to show the sound as playing
        g_bBufferPaused = FALSE;
        EnablePlayUI( hDlg, VIDEO_STATE_PLAYING);
    }
    else
    {
        if(g_pSound->IsSoundPlaying())
        {
            // To pause, just stop the buffer, but don't reset the position
            if( g_pSound )
                g_pSound->Stop();

            g_bBufferPaused = TRUE;
            SetDlgItemText( hDlg, IDC_PLAY, "Play" );
        }
        else
        {
            // The buffer is not playing, so play it again
            DWORD dwFlags = bLooped ? DSBPLAY_LOOPING : 0L;

            if(FAILED( hr = g_pSound->Play( 0, dwFlags ) ) )
                return DXTRACE_ERR( TEXT("Play"), hr );

            // Update the UI controls to show the sound as playing
            g_bBufferPaused = FALSE;
            EnablePlayUI( hDlg, VIDEO_STATE_PLAYING);
        }
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: OnTimer()
// Desc: When we think the sound is playing this periodically checks to see if 
//       the sound has stopped.  If it has then updates the dialog. This occurs at 4Hz
//		 Timer is also driving the refresh of the video which happens at 15Hz
//-----------------------------------------------------------------------------
VOID OnTimer( HWND hDlg ) 
{
	static clock_t iterationTime;
	static unsigned long ulTimerCount = 0;
	unsigned short unMin, unSec, unSubSec;
	char str[32] = { 0 };
	HRESULT hr;
	static unsigned int unAudioSyncFrame = 0;

	if (++ulTimerCount % FRAME_RATE_HZ == 0)//Check only at 4hz, when timer is at 15Hz
	{
		if (IsWindowEnabled(GetDlgItem(hDlg, IDC_STOP)))
		{
			// We think the sound is playing, so see if it has stopped yet.
			if (g_pSound->IsSoundPlaying())
			{
				// Update the UI controls to show the sound as stopped
				if(g_pMyVideo->getVideoState() == VIDEO_STATE_STOPPED)
				{
					EnablePlayUI(hDlg, VIDEO_STATE_STOPPED);
				}
				else if (g_pMyVideo->getVideoState() == VIDEO_STATE_PAUSED)
				{
					// To pause, just stop the buffer, but don't reset the position
					if (g_pSound)
						g_pSound->Stop();
					EnablePlayUI(hDlg, VIDEO_STATE_PAUSED);
				}
			}
		}
	}

	if(g_pMyVideo->getVideoState() == VIDEO_STATE_PLAYING)
	{
		if (!g_pSound->IsSoundPlaying())
		{
			// The 'play'/'pause' button was pressed
			if (FAILED(hr = OnPlaySound(hDlg)))
			{
				DXTRACE_ERR(TEXT("OnPlaySound"), hr);
				MessageBox(hDlg, "Error playing DirectSound buffer. "
					"Sample will now exit.", "DirectSound Sample",
					MB_OK | MB_ICONERROR);
				EndDialog(hDlg, IDABORT);
			}
		}

		unsigned short usCurrentFrameNo = 0;
		if ((usCurrentFrameNo = g_pMyVideo->copyVideoFrame(g_outImage)))
		{
			if (g_ASFrames.size() && unAudioSyncFrame < g_ASFrames.size())
			{
				if (usCurrentFrameNo == g_ASFrames.at(unAudioSyncFrame))
				{
					g_pSound->SetCurrentIndex((usCurrentFrameNo / FRAME_RATE_HZ) * 2 * 24000);
					unAudioSyncFrame++;
				}
			}
			else
			{
				//Snaps audio for every 150 frames 10 second (every 15 frames is 1 sec)
				if(usCurrentFrameNo % 150 == 0)
					g_pSound->SetCurrentIndex((usCurrentFrameNo / FRAME_RATE_HZ) * 2 * 24000);
			}

			//This is very we draw subsequent frames to display
			SetDIBitsToDevice(GetDC(hDlg),
				34, 20, g_outImage.getWidth(), g_outImage.getHeight(),
				0, 0, 0, g_outImage.getHeight(),
				g_outImage.getImageData(), &g_bmi, DIB_RGB_COLORS);

			unMin = floor(((usCurrentFrameNo+1) / FRAME_RATE_HZ) / 60);
			unSec = floor(((usCurrentFrameNo+1) / FRAME_RATE_HZ) % 60);
			unSubSec = ((usCurrentFrameNo+1) % FRAME_RATE_HZ);

			sprintf(str, "%02d:%02d.%02d", unMin, unSec, unSubSec);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_START), str);
		}

		if (usCurrentFrameNo == g_pMyVideo->getNoFrames() - 1)//Reached the end
		{
			//TODO: Check if it the loop checkbox is checked
			if (g_pSound)
			{
				g_pSound->Stop();
				g_pSound->Reset();
			}
			EnablePlayUI(hDlg, VIDEO_STATE_STOPPED);
		}
	}
	else if (g_pMyVideo->getVideoState() == VIDEO_STATE_BUFFERING)
	{
		EnablePlayUI(hDlg, VIDEO_STATE_BUFFERING);
	}
	else if(g_pMyVideo->getVideoState() == VIDEO_STATE_ANALYZING)
	{
		unsigned short usPer = 0;
		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETPOS, g_pMyVideo->getCurrentFrameNo(), 0);
		usPer = g_pMyVideo->getCurrentFrameNo() * 100 / g_pMyVideo->getNoFrames();
		sprintf(str, "%d%%", usPer);
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_PER), str);
	}
	else if (g_pMyVideo->getVideoState() == VIDEO_STATE_ANALYSIS_COMPLETE)
	{
		DrawThumbnails(hDlg,0);
		g_pMyVideo->stopVideo();
		EnablePlayUI(hDlg, VIDEO_STATE_ANALYSIS_COMPLETE);
		EnablePlayUI(hDlg, VIDEO_STATE_STOPPED);
	}
}//OnTimer

//-----------------------------------------------------------------------------
// Name: EnablePlayUI( hDlg,)
// Desc: Enables or disables the Play UI controls 
//-----------------------------------------------------------------------------
VOID EnablePlayUI( HWND hDlg, VIDEO_STATE_E _eVideoState )
{
	if (_eVideoState == VIDEO_STATE_PLAYING)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_LOOP_CHECK), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CORRECT_CHECK), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
		SetFocus(GetDlgItem(hDlg, IDC_STOP));

		EnableWindow(GetDlgItem(hDlg, IDC_PLAY), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_ANALYZE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BROWSE), FALSE);
		SetDlgItemText(hDlg, IDC_PLAY, "&Pause");
		//EnableWindow(GetDlgItem(hDlg, IDC_SCROLLBAR1), FALSE);
	}
	else if (_eVideoState == VIDEO_STATE_BUFFERING)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_PLAY), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BROWSE), FALSE);
		SetDlgItemText(hDlg, IDC_PLAY, "&Buffering");
		//EnableWindow(GetDlgItem(hDlg, IDC_SCROLLBAR1), FALSE);
	}
    else if(_eVideoState == VIDEO_STATE_PAUSED)
    {
        EnableWindow(   GetDlgItem( hDlg, IDC_LOOP_CHECK ), TRUE );
		EnableWindow(GetDlgItem(hDlg, IDC_CORRECT_CHECK), TRUE);
        EnableWindow(   GetDlgItem( hDlg, IDC_STOP ),       FALSE );

        EnableWindow(   GetDlgItem( hDlg, IDC_PLAY ),       TRUE );
		EnableWindow(GetDlgItem(hDlg, IDC_ANALYZE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDCANCEL), TRUE);
        SetFocus(       GetDlgItem( hDlg, IDC_PLAY ) );
		//EnableWindow(GetDlgItem(hDlg, IDC_SCROLLBAR1), FALSE);
        SetDlgItemText( hDlg, IDC_PLAY, "&Play" );
    }
	else if (_eVideoState == VIDEO_STATE_STOPPED)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_LOOP_CHECK), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_CORRECT_CHECK), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);

		EnableWindow(GetDlgItem(hDlg, IDC_PLAY), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_ANALYZE), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_STATIC_PER), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDCANCEL), TRUE);
		SetFocus(GetDlgItem(hDlg, IDC_PLAY));
		//EnableWindow(GetDlgItem(hDlg, IDC_SCROLLBAR1), FALSE);
		if(g_pMyVideo->getCurrentFrameNo() != g_pMyVideo->getNoFrames())
		{
			//Stopped by user and not on completion
			SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETPOS, 0, 0);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_PER), "0%");
		}
		SetDlgItemText(hDlg, IDC_PLAY, "&Play");
	}
	else if (_eVideoState == VIDEO_STATE_ANALYSIS_COMPLETE)
	{
		//EnableWindow(GetDlgItem(hDlg, IDC_SCROLLBAR1), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BROWSE), TRUE);
	}
	else if (_eVideoState == VIDEO_STATE_ANALYZING)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_ANALYZE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STATIC_PER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_LOOP_CHECK), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CORRECT_CHECK), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PLAY), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BROWSE), FALSE);
		//EnableWindow(GetDlgItem(hDlg, IDC_SCROLLBAR1), FALSE);
		SetFocus(GetDlgItem(hDlg, IDC_ANALYZE));
	}
}//EnablePlayUI

void DrawThumbnails(HWND _hDlg, int _nHShift)
{
	BITMAPINFO bitmapinfo;
	MyImage image;
	RECT rect;
	int nWindowWidth;

	if (GetWindowRect(_hDlg, &rect))
	{
		nWindowWidth = rect.right - rect.left;
	}

	image.setHeight(g_pMyVideo->getVideoHeight());
	image.setWidth(g_pMyVideo->getVideoWidth());

	memset(&bitmapinfo, 0, sizeof(bitmapinfo));
	bitmapinfo.bmiHeader.biSize = sizeof(g_bmi.bmiHeader);
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = 24;
	bitmapinfo.bmiHeader.biCompression = BI_RGB;
	bitmapinfo.bmiHeader.biWidth = g_pMyVideo->getVideoWidth() / 4;
	bitmapinfo.bmiHeader.biHeight = -(g_pMyVideo->getVideoHeight() / 4);  // Use negative height.  DIB is top-down.
	bitmapinfo.bmiHeader.biSizeImage = (g_pMyVideo->getVideoWidth() / 4)*(g_pMyVideo->getVideoHeight() / 4);

	if(!g_IFrames.size())
	{
		//Create I frames here
		g_IFrames = g_pMyVideo->iFramesPatch();
		//g_IFrames = g_pMyVideo->getIFrames();//First Time

		//Create summarization frames here (aflag)
		g_summarizationFrames = g_pMyVideo->summarizationFramesPatch();
		char str[64] = { 0 };
		for (int i = 0; i < g_summarizationFrames.size(); i++) {
			sprintf(str, "%d\n", g_summarizationFrames[i]);
			OutputDebugString(_T(str));
		}

		sprintf(str, "Number of I-frames: %d\n", g_IFrames.size());
		OutputDebugString(_T(str));
		sprintf(str, "Number of summarization frames: %d\n", g_summarizationFrames.size());
		OutputDebugString(_T(str));

		//Create summarization video and audio here
		g_pMyVideo->writeVectortoVideo("summarization test.rgb", g_summarizationFrames);
		WAVAudio sumAudio;
		FILE* inputAudio = fopen(AudioPath, "rb");
		sumAudio.readWAV(inputAudio);
		sumAudio.writeVectortoWAV("summarization test.wav", g_summarizationFrames);
		fclose(inputAudio);

		SetScrollRange(GetDlgItem(_hDlg, IDC_SCROLLBAR1), SB_CTL, 0, g_IFrames.size(), TRUE);
	}

	for (int i = 0; i < 4; ++i)
	{
		volatile int index = i + _nHShift;

		if (index < g_IFrames.size())
		{
			g_pMyVideo->readVideoFrame(image, g_IFrames.at(index));

			SetDIBitsToDevice(GetDC(_hDlg),
				THUMBNAIL_X_OFFSET + i*(g_pMyVideo->getVideoWidth() / 4 + 1),
				320,
				image.getWidth() / 4,
				image.getHeight() / 4,
				0, 0, 0, image.getHeight() / 4,
				image.getImageThumbnailData(), &bitmapinfo, DIB_RGB_COLORS);
		}
		else
		{
			//DeleteObject
		}
	}//for
}//DrawThumbnails

void DrawSetFrame(HWND _hDlg, int _nIndex)
{
	BITMAPINFO bitmapinfo;
	MyImage image;
	unsigned short unMin, unSec, unSubSec;
	char str[128] = { 0 };

	image.setHeight(g_pMyVideo->getVideoHeight());
	image.setWidth(g_pMyVideo->getVideoWidth());

	image = g_pMyVideo->setCurrentFrameNo(_nIndex);

	memset(&bitmapinfo, 0, sizeof(bitmapinfo));
	bitmapinfo.bmiHeader.biSize = sizeof(g_bmi.bmiHeader);
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = 24;
	bitmapinfo.bmiHeader.biCompression = BI_RGB;
	bitmapinfo.bmiHeader.biWidth = g_pMyVideo->getVideoWidth();
	bitmapinfo.bmiHeader.biHeight = -(g_pMyVideo->getVideoHeight());  // Use negative height.  DIB is top-down.
	bitmapinfo.bmiHeader.biSizeImage = g_pMyVideo->getVideoWidth()*g_pMyVideo->getVideoHeight();

	SetDIBitsToDevice(GetDC(_hDlg),
		34, 20, image.getWidth(), image.getHeight(),
		0, 0, 0, image.getHeight(),
		image.getImageData(), &g_bmi, DIB_RGB_COLORS);

	unMin = floor(((_nIndex+1) / FRAME_RATE_HZ) / 60);
	unSec = floor(((_nIndex + 1) / FRAME_RATE_HZ) % 60);
	unSubSec = ((_nIndex + 1) % FRAME_RATE_HZ);

	sprintf(str, "%02d:%02d.%02d", unMin, unSec, unSubSec);
	SetWindowText(GetDlgItem(_hDlg, IDC_STATIC_START), str);

	g_pSound->SetCurrentIndex((_nIndex / FRAME_RATE_HZ) * 2 * 24000);
}//DrawSetFrame