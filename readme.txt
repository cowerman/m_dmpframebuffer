++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
cat /dev/graphics/fb0 /data/local/tmp/fb.png

adb pull /data/local/tmp/fb.png fb.png
也可以在板子上直接使用下面的命令，也可以在ubutnu终端上执行
dd bs=14400 count=360 if=fb.png of=out1.png
此时已经可以看到framebuffer上的内容了（out1.png）
#ffmpeg -vcodec rawvideo -f rawvideo -pix_fmt rgba -s 360X360 -i xxx.raw -f image2 -vcodec png frame-%d.png

//截图直接保存到电脑
$ adb shell screencap -p | sed 's/\r$//' > screen.png

//  /dev/vcs* && /dev/vcsa*
http://blog.163.com/cmdbat%40126/blog/static/170292123201251895524238/

//actury bug from cat to bmp file
http://bbs.csdn.net/topics/390502984?page=1

//android gralloc 流程分析 
http://blog.csdn.net/g_salamander/article/details/8424334
http://www.tuicool.com/articles/FJFnMj

//bmp to rgb888 一组rgb转换函数，支持rgb565/rgb888/xrgb8888之间的数据转换
http://www.360doc.com/content/14/0826/13/14797374_404727071.shtm

//cat source code:
http://blog.csdn.net/earbao/article/details/43452591

other website of source to dump the frame:
http://www.douban.com/group/topic/43039115/
https://android.googlesource.com/platform/system/extras/+/donut-release/tests/framebuffer/fb_test.c
http://www.pocketmagic.net/android-native-screen-capture-application-using-the-framebuffer/
http://www.codeproject.com/Articles/991640/Androids-Graphics-Buffer-Management-System-Part-I


//screen shot
http://blog.csdn.net/cuiran/article/details/39494349
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
screen_cap from SurfaceFlinger.cpp service
the client form frameworks\native\services\surfaceflinger\tests\screencap

SurfaceFlinger::captureScreenImplLocked
 // get screen geometry           
uint32_t hw_w = hw->getWidth();  
uint32_t hw_h = hw->getHeight(); 
if ((reqWidth > hw_w) || (reqHeight > hw_h)) { 
	    ALOGE("size mismatch (%d, %d) > (%d, %d)", 
				            reqWidth, reqHeight, hw_w, hw_h);  
		    return BAD_VALUE;                          
}                                              
reqWidth  = (!reqWidth)  ? hw_w : reqWidth;  
reqHeight = (!reqHeight) ? hw_h : reqHeight; 

// create a surface (because we're a producer, and we need to dequeue/queue a buffer)                                    
sp<Surface> sur = new Surface(producer, false);               
ANativeWindow* window = sur.get();                            
status_t result = native_window_api_connect(window, NATIVE_WINDOW_API_EGL);
ANativeWindowBuffer* buffer;

/* TODO: Once we have the sync framework everywhere this can use
    * server-side waits on the fence that dequeueBuffer returns.   
	 */                                                             
result = native_window_dequeue_buffer_and_wait(window,  &buffer);
// create an EGLImage from the buffer so we can later turn it into a texture                                         
EGLImageKHR image = eglCreateImageKHR(mEGLDisplay, EGL_NO_CONTEXT,EGL_NATIVE_BUFFER_ANDROID, buffer, NULL);                 

// this binds the given EGLImage as a framebuffer for the duration of this scope.                                               
RenderEngine::BindImageAsFramebuffer imageBond(getRenderEngine(), image);

// this will in fact render into our dequeued buffer via an FBO, which means we didn't have to create an EGLSurface and therefore we're not dependent on the context's EGLConfig.            
renderScreenImplLocked(hw, sourceCrop, reqWidth, reqHeight, minLayerZ, maxLayerZ, true, useIdentityTransform, rotation);                                

// Attempt to create a sync khr object that can produce a sync point. If that isn't available, create a non-dupable sync object in the fallback path and wait on it directly.                                                      
EGLSyncKHR sync;                                                             
if (!DEBUG_SCREENSHOTS) {                                                    
	   sync = eglCreateSyncKHR(mEGLDisplay, EGL_SYNC_NATIVE_FENCE_ANDROID, NULL);
	      // native fence fd will not be populated until flush() is done.           
	      getRenderEngine().flush();                                                
}

// fallback path                                               
sync = eglCreateSyncKHR(mEGLDisplay, EGL_SYNC_FENCE_KHR, NULL);
if (sync != EGL_NO_SYNC_KHR) {                                 
	    EGLint result = eglClientWaitSyncKHR(mEGLDisplay, sync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, 2000000000 /*2 sec*/);
		    EGLint eglErr = eglGetError();                             


			native_window_api_disconnect(window, NATIVE_WINDOW_API_EGL);



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			jack's suport of code about vnc to dump fb
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Surfaceflinger:

http://wnwolf.com/android/2014/06/22/surfaceflinger-and-client/
http://blog.csdn.net/michaelcao1980/article/details/43233667
http://www.embedu.org/column/column339.htm
http://blog.csdn.net/yunjinwang/article/details/8072039
