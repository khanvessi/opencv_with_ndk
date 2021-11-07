#include <jni.h>
#include <time.h>
#include <android/log.h>
#include <android/bitmap.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>



#define  LOG_TAG    "libimageprocessing"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOG_E(...) 	__android_log_print(ANDROID_LOG_ERROR,APP_NAME,__VA_ARGS__)



typedef struct {
    uint8_t alpha;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
}argb;


//extern "C" JNIEXPORT jstring JNICALL
//Java_com_example_imgfilters_MainActivity_stringFromJNI(
//        JNIEnv* env,
//        jobject /* this */) {
//    std::string hello = "Hello from C++";
//    return env->NewStringUTF(hello.c_str());
//}

static int rgb_clamp(int value) {
    if (value > 255) {
        return 255;
    }
    if (value < 0) {
        return 0;
    }
    return value;
}


static void brightness(AndroidBitmapInfo *info, void *pixels, float brightnessValue) {
    int xx, yy, red, green, blue;
    uint32_t *line;

    for (yy = 0; yy < info->height; yy++) {
        line = (uint32_t *) pixels;
        for (xx = 0; xx < info->width; xx++) {

            //extract the RGB values from the pixel
            red = (int) ((line[xx] & 0x00FF0000) >> 16);
            green = (int) ((line[xx] & 0x0000FF00) >> 8);
            blue = (int) (line[xx] & 0x00000FF);

            //manipulate each value
            red = rgb_clamp((int) (red * brightnessValue));
            green = rgb_clamp((int) (green * brightnessValue));
            blue = rgb_clamp((int) (blue * brightnessValue));

            // set the new pixel back in
            line[xx] =
                    ((red << 16) & 0x00FF0000) |
                    ((green << 8) & 0x0000FF00) |
                    (blue & 0x000000FF);
        }

        pixels = (char *) pixels + info->stride;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_imgfilters_MainActivity_brightness(JNIEnv *env, jobject thiz, jobject bitmap,
                                                    jfloat brightnessValue) {

    AndroidBitmapInfo info;
    int ret;
    void *pixels;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not RGBA_8888 !");
        return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
    }

    brightness(&info, pixels, brightnessValue);

    AndroidBitmap_unlockPixels(env, bitmap);

}


/**
 * performs operations on the image to convert the image to gray
 */
void convertingToGray(void *pixelsColor, void *pixelsGrayColor,
                      AndroidBitmapInfo *infoColor, AndroidBitmapInfo *grayImageInfo) {
    int y, x;

    for (y = 0; y < infoColor->height; y++) {
        argb *line = (argb *) pixelsColor;

        // here we use uint_8 instead of uint32_t since gray images are alpha_8 images
        // that use 8 bit coloring system and not 32
        uint8_t *grayline = (uint8_t *) pixelsGrayColor;

        for (x = 0; x < infoColor->width; x++) {
            grayline[x] = 0.21 * line[x].red + 0.71 * line[x].green + 0.07 * line[x].blue;
            //grayline[x] = 0.3 * line[x].red + 0.59 * line[x].green + 0.11 * line[x].blue;
        }

        pixelsColor = (char *) pixelsColor + infoColor->stride;
        pixelsGrayColor = (char *) pixelsGrayColor + grayImageInfo->stride;
    }
}

/**
 * Operations for inverting the current image
 */
void invertImage(AndroidBitmapInfo * infogray, void * pixelsgray) {
    int x,y ;
    int red, green, blue;

    for (y=0;y<infogray->height;y++) {
        // commenting the line above since 1/4 th of the image gets converted
        // and not the whole, ARGB = 32 bits color system, while
        // uint8_t uses 8bits.(convers 8 bits)
        // uint8_t * line = (uint8_t *)pixelsgray;

        uint32_t * line = (uint32_t *)pixelsgray;

        for (x=0;x<infogray->width;x++) {

            // subtract the value from 255
            red = 255 - ((int)((line[x] & 0x00FF0000) >> 16));
            green = 255 - ((int)((line[x] & 0x0000FF00) >> 8));
            blue = 255 - ((int)((line[x] & 0x000000FF)));

            line[x] = ((red << 16) & 0x00FF0000) |
                      ((green << 8) & 0x0000FF00) |
                      (blue & 0x000000FF);
        }
        pixelsgray = (char *) pixelsgray + infogray->stride;
    }
}

void convertingImageToSepia(AndroidBitmapInfo * infoColor, void *  pixelsColor,
                            void * pixelsSepia, AndroidBitmapInfo * sepiaImageColor) {

    /*  =====================================	*/
    /*  For refrerence, SEPIA CODE
        R' = (R � 0.393 + G � 0.769 + B � 0.189);
        G' = (R � 0.349 + G � 0.686 + B � 0.168);
        B' = (R � 0.272 + G � 0.534 + B � 0.131);
    /* 	===================================== 	*/

    int y,x;

    for (y=0;y<infoColor->height;y++) {
        argb * line = (argb *) pixelsColor;
        argb * sepialine = (argb *) pixelsSepia;

        /* sepialine[x].red = 0.293 * line[x].red + 0.169* line[x].green + 0.489 * line[x].blue;
                sepialine[x].green = 0.349 * line[x].red + 0.483 * line[x].green + 0.168 * line[x].blue;
                sepialine[x].blue =  0.292 * line[x].red + 0.554 * line[x].green + 0.154 * line[x].blue;
                sepialine[x].alpha = line[x].alpha; */

        for (x=0;x<infoColor->width;x++) {
            // 0.393 * line[x].red + 0.369* line[x].green + 0.189 * line[x].blue;
            /*sepialine[x].red = 0.003 * line[x].red + 0.549* line[x].green + 0.449 * line[x].blue;
                    sepialine[x].green = 0.009 * line[x].red + 0.903 * line[x].green + 0.008 * line[x].blue;
                    sepialine[x].blue =  0.004 * line[x].red + 0.254 * line[x].green + 0.150 * line[x].blue;
                    sepialine[x].alpha = line[x].alpha;*/

            sepialine[x].red = 0.393 * line[x].red + 0.369* line[x].green + 0.189 * line[x].blue;
            sepialine[x].green = 0.349 * line[x].red + 0.483 * line[x].green + 0.168 * line[x].blue;
            sepialine[x].blue =  0.292 * line[x].red + 0.554 * line[x].green + 0.154 * line[x].blue;
            sepialine[x].alpha = line[x].alpha;
        }

        pixelsColor = (char *)pixelsColor + infoColor->stride;
        pixelsSepia = (char *) pixelsSepia + sepiaImageColor->stride;
    }
}

/**
 * Performs operations on the image to warmify it
 */
void warmifyingImage(AndroidBitmapInfo * infoColor, void * pixelsColor,
                     void * pixelsWarmColor , AndroidBitmapInfo * warmImageColor) {
    int y,x;

    for (y=0;y<infoColor->height;y++) {
        argb * line = (argb *) pixelsColor;
        argb * warmline = (argb *) pixelsWarmColor;

        /* 	greenline[x].red = 0 * line[x].red + 0.5* line[x].green + 0 * line[x].blue;
            greenline[x].green = 0.803 * line[x].green + 0.08 * line[x].blue; //0.009 * line[x].red + 0.803 * line[x].green + 0.008 * line[x].blue;
            greenline[x].blue =  0.0;
            greenline[x].alpha = line[x].alpha;

            greenline[x].red = 0.5* line[x].red;// 0 * line[x].red + 0.02* line[x].green + 0 * line[x].blue;
            greenline[x].green = line[x].green;//0 * line[x].red + 1.0 * line[x].green + 0.0 * line[x].blue; //0.009 * line[x].red + 0.803 * line[x].green + 0.008 * line[x].blue;
            greenline[x].blue =0* line[x].blue;
            greenline[x].alpha = line[x].alpha;
         */
        for (x=0;x<infoColor->width;x++) {
            // 0 * line[x].red + 0.02* line[x].green + 0 * line[x].blue;
            warmline[x].red = 0.65 * line[x].red;

            //0 * line[x].red + 1.0 * line[x].green + 0.0 * line[x].blue; //0.009 * line[x].red + 0.803 * line[x].green + 0.008 * line[x].blue;
            warmline[x].green = 1.098 * line[x].green;

            //1.1 * line[x].blue;
            warmline[x].blue =0;

            warmline[x].alpha = line[x].alpha;
        }

        pixelsColor = (char *)pixelsColor + infoColor->stride;
        pixelsWarmColor = (char *) pixelsWarmColor + warmImageColor->stride;
    }
}

/**
 * Performs operations to convert an image into red scale
 */
void convertingImageToRed(AndroidBitmapInfo * infoColor, void * pixelsColor,
                          void * pixelsRedColor, AndroidBitmapInfo * redImageInfo) {
    int y,x;

    for (y=0;y<infoColor->height;y++) {
        argb * greyline = (argb *) pixelsColor;
        argb * sepialine = (argb *) pixelsRedColor;

        for (x=0;x<infoColor->width;x++) {
            sepialine[x].red =  0.1 *greyline[x].red;
            sepialine[x].green = 0.9;
            sepialine[x].blue = 0;
            sepialine[x].alpha = greyline[x].alpha;
        }

        pixelsColor = (char *)pixelsColor + infoColor->stride;
        pixelsRedColor = (char *) pixelsRedColor + redImageInfo->stride;
    }
}



extern "C"
JNIEXPORT void JNICALL
Java_com_example_imgfilters_MainActivity_convertToGray(JNIEnv *env, jobject jobj, jobject bitmapIn,
                                                       jobject bitmapOut) {

    // the variables needed declared here
    AndroidBitmapInfo infoColor;
    void *pixelsColor;
    AndroidBitmapInfo grayImageInfo;
    void *pixelsGrayColor;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmapIn, &infoColor)) < 0) {
        //LOG_E("AndroidBitmap_getInfo() failed ! error=%d", ret);
        //env->ThrowNew(cls, "AndroidBitmap_getInfo() bitmapIn failed !");
        return;
    }

    if ((ret = AndroidBitmap_getInfo(env, bitmapOut, &grayImageInfo)) < 0) {
       // LOG_E("AndroidBitmap_getInfo() failed ! error=%d", ret);
        //env->ThrowNew(cls, "AndroidBitmap_getInfo() bitmapOut failed !");
        return;
    }

    if (infoColor.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        //LOG_E("Bitmap format is not RGBA_8888 !");
        //env->ThrowNew(cls, "Bitmap format is not RGBA_8888 !");
        return;
    }


    if (grayImageInfo.format != ANDROID_BITMAP_FORMAT_A_8) {
        //LOG_E("Bitmap format is not A_8 !");
        //env->ThrowNew(cls, "Bitmap format is not A_8 !");
        return;
    }

    //COFFEE_TRY() {
    if ((ret = AndroidBitmap_lockPixels(env, bitmapIn, &pixelsColor)) < 0) {
       // LOG_E("AndroidBitmap_lockPixels() failed ! error=%d", ret);
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmapOut, &pixelsGrayColor)) < 0) {
       // LOG_E("AndroidBitmap_lockPixels() failed ! error=%d", ret);
    }

    convertingToGray(pixelsColor, pixelsGrayColor, &infoColor, &grayImageInfo);

    AndroidBitmap_unlockPixels(env, bitmapIn);
    AndroidBitmap_unlockPixels(env, bitmapOut);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_imgfilters_MainActivity_invertImage(JNIEnv *env, jobject jobj, jobject bitmapIn) {
    AndroidBitmapInfo  	infogray;
    void*              	pixelsgray;
    int                	ret;
    int 				y;
    int             	x;
    int					red, green, blue;
    uint8_t 			save;

    if ((ret = AndroidBitmap_getInfo(env, bitmapIn, &infogray)) < 0) {
        //LOG_E("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmapIn, &pixelsgray)) < 0) {
       // LOG_E("AndroidBitmap_lockPixels() failed ! error=%d", ret);
    }

    invertImage(&infogray, pixelsgray);
    AndroidBitmap_unlockPixels(env, bitmapIn);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_imgfilters_MainActivity_convertToSepia(JNIEnv *env, jobject jobj,
                                                        jobject bitmapIn, jobject bitmapOut) {

    AndroidBitmapInfo 	infoColor;
    void*				pixelsColor;
    AndroidBitmapInfo	sepiaImageColor;
    void*				pixelsSepia;
    int					ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmapIn, &infoColor)) < 0) {

        return;
    }

    if ((ret = AndroidBitmap_getInfo(env, bitmapOut, &sepiaImageColor)) < 0) {
        return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmapIn, &pixelsColor)) < 0) {
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmapOut, &pixelsSepia)) < 0) {
    }
//
//    COFFEE_TRY() {
        convertingImageToSepia(&infoColor, pixelsColor, pixelsSepia, &sepiaImageColor);
        AndroidBitmap_unlockPixels(env,bitmapIn);
        AndroidBitmap_unlockPixels(env, bitmapOut);
    //}COFFEE_CATCH() {
       // LOG_E("error : %s", coffeecatch_get_message());
   // }COFFEE_END();

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_imgfilters_MainActivity_warmifyImage(JNIEnv *env, jobject jobj, jobject bitmapIn,
                                                      jobject bitmapOut) {
    AndroidBitmapInfo 	infoColor;
    void*				pixelsColor;
    AndroidBitmapInfo	warmImageColor;
    void*				pixelsWarmColor;
    int					ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmapIn, &infoColor)) < 0) {
        return;
    }

    if ((ret = AndroidBitmap_getInfo(env, bitmapOut, &warmImageColor)) < 0) {
        return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmapIn, &pixelsColor)) < 0) {
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmapOut, &pixelsWarmColor)) < 0) {
    }

//    COFFEE_TRY() {
        warmifyingImage(&infoColor, pixelsColor, pixelsWarmColor, &warmImageColor);
        AndroidBitmap_unlockPixels(env,bitmapIn);
        AndroidBitmap_unlockPixels(env, bitmapOut);
//    }COFFEE_CATCH() {
//       // LOG_E("error : %s", coffeecatch_get_message());
//    }COFFEE_END();

}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_imgfilters_MainActivity_convertToRed(JNIEnv *env, jobject thiz, jobject bitmapIn,
                                                      jobject bitmapOut) {
    // the variables needed declared here
    AndroidBitmapInfo 	infoColor;
    void*				pixelsColor;
    AndroidBitmapInfo	redImageInfo;
    void*				pixelsRedColor;
    int					ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmapIn, &infoColor)) < 0) {
        return;
    }

    if ((ret = AndroidBitmap_getInfo(env, bitmapOut, &redImageInfo)) < 0) {
        return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmapIn, &pixelsColor)) < 0) {
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmapOut, &pixelsRedColor)) < 0) {
    }

//    COFFEE_TRY() {
        convertingImageToRed(&infoColor, pixelsColor, pixelsRedColor, &redImageInfo);
        AndroidBitmap_unlockPixels(env,bitmapIn);
        AndroidBitmap_unlockPixels(env, bitmapOut);
//    }COFFEE_CATCH() {
//        LOG_E("error : %s", coffeecatch_get_message());
//    }COFFEE_END();
}