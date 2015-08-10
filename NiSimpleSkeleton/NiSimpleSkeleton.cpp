
/****************************************************************************
*                                                                           *
*  OpenNI 1.x Alpha                                                         *
*  Copyright (C) 2011 PrimeSense Ltd.                                       *
*                                                                           *
*  This file is part of OpenNI.                                             *
*                                                                           *
*  OpenNI is free software: you can redistribute it and/or modify           *
*  it under the terms of the GNU Lesser General Public License as published *
*  by the Free Software Foundation, either version 3 of the License, or     *
*  (at your option) any later version.                                      *
*                                                                           *
*  OpenNI is distributed in the hope that it will be useful,                *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the             *
*  GNU Lesser General Public License for more details.                      *
*                                                                           *
*  You should have received a copy of the GNU Lesser General Public License *
*  along with OpenNI. If not, see <http://www.gnu.org/licenses/>.           *
*                                                                           *
****************************************************************************/
/*libfreenect: https://github.com/OpenKinect/libfreenect
 * OpenNI:     https://github.com/OpenNI/OpenNI
 *       /kinect/OpenNI/Samples/NiSimpleSkeleton
 * */

/* OPENNI sensors are listed below
Microsoft Kinect
ASUS Xtion PRO (no RGB)
ASUS Xtion PRO Live (with RGB)
PrimeSense PSDK 5.0 
Intel realsense    https://github.com/teknotus/depthview
*RGB-D sensor*/

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include "SceneDrawer.h"
#include <XnPropNames.h>
#include <math.h> 
#include <sys/shm.h> //shared memory
#include <sys/stat.h>
#include <sys/ipc.h>
//---------------------------------------------------------------------------
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h> //shared memory 
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
//-------------------------------
#include <iostream>
#include <fstream>
#define SHMSZ     37//size
// Defines
// Globals
//---------------------------------------------------------------------------
//xn::Context g_Context;
//xn::ScriptNode g_scriptNode;
//xn::DepthGenerator g_DepthGenerator;
//xn::UserGenerator g_UserGenerator;
xn::Player g_Player;

//XnBool g_bNeedPose = FALSE;
//XnChar g_strPose[20] = "";
XnBool g_bDrawBackground = TRUE;
XnBool g_bDrawPixels = TRUE;
XnBool g_bDrawSkeleton = TRUE;
XnBool g_bPrintID = TRUE;
XnBool g_bPrintState = TRUE;

XnBool g_bPrintFrameID = FALSE;
XnBool g_bMarkJoints = FALSE;
using namespace std;
#ifndef USE_GLES
#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif
#else
	#include "opengles.h"
#endif

#ifdef USE_GLES
static EGLDisplay display = EGL_NO_DISPLAY;
static EGLSurface surface = EGL_NO_SURFACE;
static EGLContext context = EGL_NO_CONTEXT;
#endif

#define GL_WIN_SIZE_X 720
#define GL_WIN_SIZE_Y 480

XnBool g_bPause = false;
XnBool g_bRecord = false;

XnBool g_bQuit = false;

//---------------------------------------------------------------------------

#define PI 3.14159265
//---------------------------------------------------------------------------
#define SAMPLE_XML_PATH "../../Config/SamplesConfig.xml"
#define SAMPLE_XML_PATH_LOCAL "SamplesConfig.xml"

//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
xn::Context g_Context;
xn::ScriptNode g_scriptNode;
xn::DepthGenerator g_DepthGenerator;
xn::UserGenerator g_UserGenerator;

XnBool g_bNeedPose = FALSE;
XnChar g_strPose[20] = "";

#define MAX_NUM_USERS 15
//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
//--------------------------------------------------
// Communication
	//~ int rc1; // For the returned values
	//~ pthread_t thread1; // Threads pointer's

	// create socket
	//~ if ( (connectionFd = socket( AF_INET, SOCK_STREAM, 0 )) == -1 ) {
		//~ printf("Error creating socket\n");
		//~ exit(1);	
	//~ }
	//~ 
		//~ // set socket port options
	//~ if ( setsockopt(connectionFd, SOL_SOCKET, SO_REUSEADDR, &_true, sizeof(int)) == -1 ) {
		//~ printf("Error setting socket port options\n");		
		//~ exit(1);
	//~ }
	//~ 
	//~ // initialise address
	//~ memset( &servaddr, 0, sizeof(servaddr) ); // fill with 0's
	//~ servaddr.sin_family = PF_INET; // IPv4
	//~ servaddr.sin_port = htons( MY_PORT ); // port
	//~ servaddr.sin_addr.s_addr = inet_addr( "137.189.100.100" ); // host to network long
//~ 
//~ 



//---------------------------------------------------
XnBool fileExists(const char *fn)
{
	XnBool exists;
	xnOSDoesFileExist(fn, &exists);
	return exists;
}

//------------------------------------------------reading process ID from /proc
pid_t get_pid_from_proc_self ()
{
char target[32];
int pid;
/* Read the target of the symbolic link. */
readlink ("/proc/self", target, sizeof (target));
/* The target is a directory named for the process ID.
 */
sscanf (target, "%d", &pid);
return (pid_t) pid;
}
//------------------------------------------------------------------------------


// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d New User %d\n", epochTime, nId);
    // New user found
    if (g_bNeedPose)
    {
        g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
    }
    else
    {
        g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
    }
}
// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Lost user %d\n", epochTime, nId);	
}
// Callback: Detected a pose
void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& /*capability*/, const XnChar* strPose, XnUserID nId, void* /*pCookie*/)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Pose %s detected for user %d\n", epochTime, strPose, nId);
    g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
    g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}
// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& /*capability*/, XnUserID nId, void* /*pCookie*/)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Calibration started for user %d\n", epochTime, nId);
}

void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& /*capability*/, XnUserID nId, XnCalibrationStatus eStatus, void* /*pCookie*/)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    if (eStatus == XN_CALIBRATION_STATUS_OK)
    {
        // Calibration succeeded
        printf("%d Calibration complete, start tracking user %d\n", epochTime, nId);		
        g_UserGenerator.GetSkeletonCap().StartTracking(nId);
    }
    else
    {
        // Calibration failed
        printf("%d Calibration failed for user %d\n", epochTime, nId);
        if(eStatus==XN_CALIBRATION_STATUS_MANUAL_ABORT)
        {
            printf("Manual abort occured, stop attempting to calibrate!");
            return;
        }
        if (g_bNeedPose)
        {
            g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
        }
        else
        {
            g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
}


#define CHECK_RC(nRetVal, what)					    \
    if (nRetVal != XN_STATUS_OK)				    \
{								    \
    printf("%s failed: %s\n", what, xnGetStatusString(nRetVal));    \
    return nRetVal;						    \
}
//void glutKeyboard (unsigned char key, int /*x*/, int /*y*/)
	/*	{
			switch (key)
			{
			
				case 't':
				// Draw background?
				tilt = !tilt;
				break;
				case 'p':
				// Draw background?
				pan = !pan;
				break;
				case 'z':
				// Draw background?
				zoom = !zoom;
				break;
             }
		 } */
	XnUserID aUsers[MAX_NUM_USERS];
    XnUInt16 nUsers;
    XnSkeletonJointTransformation torsoJoint;
    XnSkeletonJointTransformation neckJoint;
    XnSkeletonJointTransformation headJoint;
    XnSkeletonJointTransformation rightshoulder;
    XnSkeletonJointTransformation leftshoulder;
    XnSkeletonJointOrientation headorientation;
    //glutCreateWindow ("User Tracker Viewer");
int main()
{
	ofstream myfile ("xxx.output"); //filename  /home/chen/kinect/OpenNI/Samples/Bin/x64-Release
    XnStatus nRetVal = XN_STATUS_OK;
    xn::EnumerationErrors errors;
    unsigned char key;
	bool tilt = 0;   //definitions of signal switches
	bool pan  = 0;
	bool zoom = 0;
	bool zoomin_1 = 0;
	bool zoomin_0 = 0;
	bool zoomout_1 = 0;
	bool PIDcount = 1;
	double directx = 0.00;
	double directy = 0.00; 
	double directz = 0.00;
	double direct_x;
                    double torsojointz;
                    double neckjointz;
                    double headjointz;
                    double der_one_headjoint = 0.00;
                    double der_two_headjoint = 0.00;
                    double der_one_headjoint_p = 0.00;
                    double headcenter = 0.00;
                    
                    
                    
                    
                    
    //-------for shared memory--------
    char c, m;
    int shmid;
    int shmidm;
    key_t keym;
    key_t keymecha; //key for mechanical robot
    char *shm, *shmm, *s, *mecha;
    keym = 1234;
    keymecha = 4321;
        /*
     * Create the segment.
     */
    if ((shmid = shmget(keym, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    
    if ((shmidm = shmget(keymecha, SHMSZ, IPC_CREAT | 0666)) < 0) { // segment for medical robot
        perror("shmget");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = (char*)shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

	if ((shmm = (char*)shmat(shmidm, NULL, 0)) == (char *) -1) {  // attaching segment for medical robot
        perror("shmat");
        exit(1);
    }
    /*
     * Now put some things into the memory for the
     * other process to read.
     */
    s = shm;
	*s = 't';
    mecha = shmm;
    *mecha = 'n';
    //-------for shared memory----------
    
    
    
    
    
    
    
    
    
    int head_up = 0;
	int count = 0;
	int counter = 0;
    const char *fn = NULL;
    if    (fileExists(SAMPLE_XML_PATH)) fn = SAMPLE_XML_PATH;
    else if (fileExists(SAMPLE_XML_PATH_LOCAL)) fn = SAMPLE_XML_PATH_LOCAL;
    else {
        printf("Could not find '%s' nor '%s'. Aborting.\n" , SAMPLE_XML_PATH, SAMPLE_XML_PATH_LOCAL);
        return XN_STATUS_ERROR;
    }
    printf("Reading config from: '%s'\n", fn);

    nRetVal = g_Context.InitFromXmlFile(fn, g_scriptNode, &errors);
    if (nRetVal == XN_STATUS_NO_NODE_PRESENT)
    {
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        printf("%s\n", strError);
        return (nRetVal);
    }
    else if (nRetVal != XN_STATUS_OK)
    {
        printf("Open failed: %s\n", xnGetStatusString(nRetVal));
        return (nRetVal);
    }

    nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
    CHECK_RC(nRetVal,"No depth");

    nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);
    if (nRetVal != XN_STATUS_OK)
    {
        nRetVal = g_UserGenerator.Create(g_Context);
        CHECK_RC(nRetVal, "Find user generator");
    }

    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected;
    if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
    {
        printf("Supplied user generator doesn't support skeleton\n");
        return 1;
    }
    nRetVal = g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
    CHECK_RC(nRetVal, "Register to user callbacks");
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, NULL, hCalibrationStart);
    CHECK_RC(nRetVal, "Register to calibration start");
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, NULL, hCalibrationComplete);
    CHECK_RC(nRetVal, "Register to calibration complete");

    if (g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
    {
        g_bNeedPose = TRUE;
        if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
        {
            printf("Pose required, but not supported\n");
            return 1;
        }
        nRetVal = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, NULL, hPoseDetected);
        CHECK_RC(nRetVal, "Register to Pose Detected");
        g_UserGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);
    }

    g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

    nRetVal = g_Context.StartGeneratingAll();
    CHECK_RC(nRetVal, "StartGenerating");

    //XnUserID aUsers[MAX_NUM_USERS];
    //XnUInt16 nUsers;
    //XnSkeletonJointTransformation torsoJoint;
    //XnSkeletonJointTransformation neckJoint;
    //XnSkeletonJointTransformation headJoint;
    //XnSkeletonJointOrientation headorientation;
   printf("Starting to run\n");
    if(g_bNeedPose)
    {
        printf("Assume calibration pose\n");
    }
    int k = 0;
    float xr;
    float yr;
    float zr;
    float xa;
    float ya;
    float za;
	//while (!xnOSWasKeyboardHit())    /////////////////modify here!
	while (1)  // the super loop begins here!!
    {
		
        g_Context.WaitOneUpdateAll(g_UserGenerator);
        // print the torso information for the first user already tracking
        nUsers=MAX_NUM_USERS;
        g_UserGenerator.GetUsers(aUsers, nUsers);  //generate a user
        for(XnUInt16 i=0; i<nUsers; i++)
        {
            if(g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])==FALSE)
                continue; //if the condition is satisfied, the following commands should never be generated.
						  //"Any remaining statements in the current iteration are not executed. "
			/*
  XN_SKEL_HEAD          = 1,    XN_SKEL_NECK            = 2,
  XN_SKEL_TORSO         = 3,    XN_SKEL_WAIST           = 4,
  XN_SKEL_LEFT_COLLAR        = 5,    XN_SKEL_LEFT_SHOULDER        = 6,
  XN_SKEL_LEFT_ELBOW        = 7,  XN_SKEL_LEFT_WRIST          = 8,
  XN_SKEL_LEFT_HAND          = 9,    XN_SKEL_LEFT_FINGERTIP    =10,
  XN_SKEL_RIGHT_COLLAR    =11,    XN_SKEL_RIGHT_SHOULDER    =12,
  XN_SKEL_RIGHT_ELBOW        =13,  XN_SKEL_RIGHT_WRIST          =14,
  XN_SKEL_RIGHT_HAND      =15,    XN_SKEL_RIGHT_FINGERTIP    =16,
  XN_SKEL_LEFT_HIP          =17,    XN_SKEL_LEFT_KNEE            =18,
  XN_SKEL_LEFT_ANKLE        =19,  XN_SKEL_LEFT_FOOT            =20,
  XN_SKEL_RIGHT_HIP          =21,    XN_SKEL_RIGHT_KNEE          =22,
  XN_SKEL_RIGHT_ANKLE        =23,    XN_SKEL_RIGHT_FOOT          =24    
			 * 
			 * */
            g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i],XN_SKEL_TORSO,torsoJoint);
            g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i],XN_SKEL_NECK,neckJoint);
            g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i],XN_SKEL_HEAD,headJoint);
            g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i],XN_SKEL_LEFT_SHOULDER,leftshoulder);
            g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i],XN_SKEL_RIGHT_SHOULDER,rightshoulder);
            g_UserGenerator.GetSkeletonCap().GetSkeletonJointOrientation(aUsers[i], XN_SKEL_HEAD, headorientation);
            //---------------------------------------------------
            
            /*
            while(k < 50)
            {
            xr += torsoJoint.position.position.X;
            yr += torsoJoint.position.position.Y;
            zr += torsoJoint.position.position.Z;
            
            xa = xr/50.0;
            ya = yr/50.0;
            za = zr/50.0;
            k++;
		    }*/
		    //---------------------------------------------------- datas from different parts of a human body
			   //~ printf("user %d: torso at (%6.2f,%6.2f,%6.2f)\n",aUsers[i],
																//~ torsoJoint.position.position.X,
																//~ torsoJoint.position.position.Y,
																//~ torsoJoint.position.position.Z);
						//~ 
				//~ printf("user %d: neck at (%6.2f,%6.2f,%6.2f)\n",aUsers[i],
																//~ neckJoint.position.position.X,
																//~ neckJoint.position.position.Y,
																//~ neckJoint.position.position.Z);
				//~ printf("user %d: head at (%6.2f,%6.2f,%6.2f)\n",aUsers[i],
																//~ headJoint.position.position.X,
																//~ headJoint.position.position.Y,
																//~ headJoint.position.position.Z);
                //~ printf("user %d: head direction x at (%6.2f,%6.2f,%6.2f)\n",aUsers[i],
                                                                //~ headorientation.orientation.elements[0],
                                                                //~ headorientation.orientation.elements[3],
                                                                //~ headorientation.orientation.elements[6]);
                //~ printf("user %d: head direction y at (%6.2f,%6.2f,%6.2f)\n",aUsers[i],
                                                                //~ headorientation.orientation.elements[1],
                                                                //~ headorientation.orientation.elements[4],
                                                                //~ headorientation.orientation.elements[7]);
                //~ printf("user %d: head direction z at (%6.2f,%6.2f,%6.2f)\n",aUsers[i],
                                                                //~ headorientation.orientation.elements[2],
                                                                //~ headorientation.orientation.elements[5],
                                                                //~ headorientation.orientation.elements[8]);
                //-----------------------------------------print process ID
                                                                
                if(PIDcount = 1)
                {
					ofstream pidfile ("/home/chen/numbers.txt"); 
					if(pidfile.is_open())
					{
						pidfile << (int) getpid ();
						pidfile.close();
						}
						PIDcount = 0;
						//return 0;
					}
                //-----------------------------------------print process ID
                
                //~ printf ("/proc/self reports process id %d\n", (int) get_pid_from_proc_self ());
					//~ 
				//~ printf ("getpid() reports process id %d\n", (int) getpid ());
				//-----------------------------------------print process ID
                    //~ double direct_x;
                    //~ double torsojointz;
                    //~ double neckjointz;
                    //~ double headjointz;
                    
                    
                    
                    torsojointz = torsoJoint.position.position.Z;
                    neckjointz = neckJoint.position.position.Z;
                    //~ headjointz = headJoint.position.position.Z;
                    if(count < 10)   //calculate first derivative of head position
                    {
						count++;
						
						}
					else
					
					{
						der_one_headjoint = headJoint.position.position.Z - headjointz;// calculate the first derivative each 10Hz
						der_two_headjoint = der_one_headjoint - der_one_headjoint_p;
						headjointz = headJoint.position.position.Z;//update headposition container
						der_one_headjoint_p = der_one_headjoint;//update head velocity container
						count = 0;
						
						} 
                   ///////////////(1)/bond setting 
                   
                   
                   //----------------set up headcenter in z axis
                  
                   if(counter < 201)   //set up headcenter in z axis
                    {
						counter++;
						if (counter == 200)
						{
						headcenter = headJoint.position.position.Z;
						}
					}
						else
					{
							//--------------trick
							counter = 300;
							if (*s == 'a')
							{
								counter =0;
								system("espeak 'activated'");
								system("espeak 'resetting reference'");
								*s = 't';
								}
							//--------------trick
					}
					
					//----------------set up headcenter in z axis
					
					
					
					if(counter ==300) // counter == 300 means reference resetted
					{		
					if (headJoint.position.position.Z - headcenter > 50)
					{
						head_up = 100;
						printf("zooming out\n");
						*mecha = 'o';
						//zoomout_1 = 1;
						zoomin_1 = 0;
					}
					
					if (((headJoint.position.position.Z - headcenter) < 50) && ((headJoint.position.position.Z - headcenter) > -50))
					{
						//head_up = 100;
						//printf("zooming out\n");
						*mecha = 'n';
						//zoomout_1 = 1;
						//zoomin_1 = 0;
					}
					
					if (headJoint.position.position.Z - headcenter < -50)
					{
						head_up = -100;
						printf("zooming in\n");
						*mecha = 'i';
						zoomin_1 = 1;
						//zoomout_1 = 0;
					}
					}
                    //double directx;
					//param = 0.5;
					
					////////////(filters)//
					direct_x = acos (headorientation.orientation.elements[6]) * 180.0 / PI;
					if (direct_x - directx > 10)  // filters: filter the noises of the data.
					
					{
						
						direct_x = directx + 10;
						directx = direct_x;
						}
					////////////(filters)//
					
					
					
					//~ printf ("The arc cosine of panning angle is %f degrees.\n",  direct_x);
					//
					printf ("reference center in z axis is %f.\n",  headcenter);
					printf ("The head's position in z axis is %f.\n",  headJoint.position.position.Z);
					
					
					    //ofstream myfile ("example.txt"); /home/chen/kinect/OpenNI/Samples/Bin/x64-Release

						//return 0;
					
					
					
					//return 0;
					double direct_y;
					//double directy;
					//param = 0.5;
					direct_y = acos (headorientation.orientation.elements[1]) * 180.0 / PI;
					if (direct_y - directy > 10)
					
					{
						
						direct_y = directy + 10;
						directy = direct_y;
						}
					//~ printf ("The arc cosine of zooming angle is %f degrees.\n",  direct_y);
					//~ //return 0;
					double direct_z;
					
					//double directz;
					//param = 0.5;
					direct_z = acos (headorientation.orientation.elements[7]) * 180.0 / PI;
					if (direct_z - directz > 10)
					
					{
						
						direct_z = directz + 10;
						directz = direct_z;
						}
					//~ printf ("The arc cosine of tilting angle is %f degrees.\n",  direct_z);
					//~ //return 0;
					  if (myfile.is_open())
  {
	//  return 0;
    //myfile << "This is element 6.\n";
    //myfile << "This is another line.\n";
    myfile <<headcenter<< "  "<<rightshoulder.position.position.Z<< "  "<< headJoint.position.position.Z<< "  "<<head_up<< "  "<<der_two_headjoint<< "  "<<der_one_headjoint<< "  "<< direct_x << "  " << direct_y << "  " <<direct_z << "  " << torsojointz << "  " << neckjointz << "  "<< headjointz <<"  .\n";
    
    //~ myfile <<  1400  << "  " << headJoint.position.position.Z  <<  "  .\n";
    
    //~ myfile <<headcenter<< "  "<<head_up<< "  .\n";
    //myfile << "This is a line.\n";
    //myfile << "This is a line.\n";
    //myfile << "This is a line.\n";
    //myfile << "This is a line.\n";
    //myfile.close();
  }
					//glutKeyboard();
					if (xnOSWasKeyboardHit())
					{
					char key = xnOSReadCharFromInput();
					//if(27==c)break;
					switch (key)
			
			{
				case 116:
				// -----------press t
				tilt = !tilt;
				break;
				case 112:
				// -----------press p
				pan = !pan;
				break;
				case 122:
				// -----------press z
				zoom = !zoom;
				break;
             }
					
			}		
		    while(pan)
		    {                                              
            if(headorientation.orientation.elements[6] > 0.5)
			{
				
				printf("yawing  ----->\n");
				
			}
			else if(headorientation.orientation.elements[6] < -0.5)
			{
				printf("yawing  <-----\n");
			}
			else
			{
			}
			break;
				}
			while(tilt)
			{
			if(headorientation.orientation.elements[1] > 0.5)
			{
				printf("rolling  right\n");
			}
			else if(headorientation.orientation.elements[1] < -0.5)
			{
				printf("rolling  left\n");
			}
			else
			{
				}
				break;
			}
			while(zoom)
			{
			if(direct_z < 60.0)
			{
				
				zoomin_0 = 0;
				zoomout_1 = 1;
				printf("zooming  out\n");
			}
			else if(direct_z > 120.0)
			{
				printf("zooming  in\n");
				zoomin_0 = 1;
			}
			else
			{
				zoomout_1 = 0;
				}
			//~ if (zoomin_1 && zoomin_0 && !zoomout_1)
			//~ {
				//~ printf("zooming  in\n");
				//~ }
			//~ if(!zoomin_1 && !zoomin_0 && zoomout_1)
			//~ {
				//~ printf("zooming  out\n");
				//~ }
				break;
			}	
			
            /*
            if(torsoJoint.position.position.X < (0.0 - 50.0))
			{
				printf("xxxxxxxxxxxx  down\n");
			}
			else if(torsoJoint.position.position.X > (0.0 + 50.0))
		   //else if((xa - 50.0) <= torsoJoint.position.position.X <= (xa + 50.0))
			{
				printf("xxxxxxxxxxxx  up\n");
			}
			//if(torsoJoint.position.position.X > (xa + 50.0))
			else
			{
				//printf("xxxxxxxxxxxx  up\n");
			}
			 if(torsoJoint.position.position.Y < (0.0 - 50.0))
			{
				printf("yyyyyyyyyyyy  down\n");
			}
		    else if(torsoJoint.position.position.Y > (0.0 + 50.0))
			{
				printf("yyyyyyyyyyyy  up\n");
			}
			//if(torsoJoint.position.position.Y > (ya + 50.0))
			else
			{
				//printf("yyyyyyyyyyyy  up\n");
			}
			 if(torsoJoint.position.position.Z < (1000.0 - 50.0))
			{
				printf("zzzzzzzzzzzz  down\n");
			}
			else if(torsoJoint.position.position.Z > (1000.0 + 50.0))
			{
				printf("zzzzzzzzzzzz  up\n");
			}
		  else //if((za - 50.0) <= torsoJoint.position.position.Z <= (za + 50.0))
			{
				//printf("zzzzzzzzzzzz  hold\n");
			}
			//if(torsoJoint.position.position.Z > (za + 50.0))
			*/
			
			
	
	//---------------------------------------------------------------------------------------------------------//		
										//find the function and get use -> SceneDrawer.cpp-> draw joint->line :189
										//SceneDrawer.cpp-> if (g_bMarkJoints) ->line:452      
										//SceneDrawer.cpp-> GetSkeletonJointPosition->line:203 
										//SceneDrawer.cpp->extern xn::UserGenerator g_UserGenerator;->g_UserGenerator 
										//XnCppWraper.h->class Generator->line:3325 
										//SceneDrawer.cpp-> joint position line:211
										//SceneDrawer.cpp-> draw circle line:215
										//xntypes.h ->joint structure->line:590 
	//---------------------------------------------------------------------------------------------------------//			
			
        }
        
    }
    g_scriptNode.Release();
    g_DepthGenerator.Release();
    g_UserGenerator.Release();
    g_Context.Release();

}
	
