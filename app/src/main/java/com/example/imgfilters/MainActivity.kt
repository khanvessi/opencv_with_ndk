package com.example.imgfilters

import android.content.ContentValues
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.Drawable
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Handler
import android.util.Log
import android.view.View
import android.widget.*
import android.widget.SeekBar.OnSeekBarChangeListener
import com.example.imgfilters.databinding.ActivityMainBinding
import org.opencv.android.Utils
import org.opencv.core.CvType
import org.opencv.core.Mat

class MainActivity : AppCompatActivity() {

    /**
     * A native method that is implemented by the 'imgfilters' native library,
     * which is packaged with this application.
     */
    //external fun stringFromJNI(): String

    companion object {
        // Used to load the 'imgfilters' library on application startup.
        init {
            System.loadLibrary("imgfilters")
        }
    }

    private lateinit var binding: ActivityMainBinding
    private var brightness = 0f
    private var original: Bitmap? = null
    private var mBmp: Bitmap? = null
    private var mImgDrawable: Drawable? = null
    private val mRelLayout: RelativeLayout? = null
    private var width = 0
    private  var height: Int = 0
    private val brightnessValue = 0f

    //JNI FUNCTIONS
    external fun brightness(bmp: Bitmap?, brightness: Float)
    external fun convertToGray(bitmapIn: Bitmap?, bitmapOut: Bitmap?)
    external fun invertImage(bitmapIn: Bitmap?)
    external fun warmifyImage(bitmapIn: Bitmap?, bitmapOut: Bitmap?)
    external fun convertToRed(bitmapIn: Bitmap?, bitmapOut: Bitmap?)

    //using opencv
    external fun ImgToGray(addrRgba: Long, addrGray: Long): String



    @Throws(MyException::class)
    external fun convertToSepia(bitmapIn: Bitmap?, bitmapOut: Bitmap?)


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        /**
         * CLICK LISTENNERS
         */
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        original = BitmapFactory.decodeResource(resources, R.mipmap.test)
        binding.imageView!!.setImageBitmap(original)
        //binding.button.setOnClickListener { adjustBrightness() }
        binding.btnReset.setOnClickListener {
            binding.imageView.setImageBitmap(original)
            binding.seekBar.progress = 0
            brightness = 0.0f
            adjustBrightness()
            binding.txtFilterApplied.text = "No Filter"
        }

        binding.imgGrey.setOnClickListener(View.OnClickListener {
            convertImageToGray()
            binding.txtFilterApplied.text = "GREY"
        })

        binding.imgInvert.setOnClickListener(View.OnClickListener {
            invertImage()
            binding.txtFilterApplied.text = "INVERT"
        })

        binding.imgSepia.setOnClickListener(View.OnClickListener {
            //invertImage()
            convertImageToSepia()
            binding.txtFilterApplied.text = "SEPIA"
        })

//        binding.btnWarmify.setOnClickListener(View.OnClickListener {
//            //invertImage()
//            warmifyImage()
//        })

//        binding.btnRed.setOnClickListener(View.OnClickListener {
//            convertImageToRed()
//        })


//        if(OpenCVLoader.initDebug()){
//            Log.e(ContentValues.TAG, "successfully: ", )
//        }else{
//            Log.e(ContentValues.TAG, "Failed: ", )
//        }

        // Example of a call to a native method
//        binding.sampleText.text = stringFromJNI()

        //TOGREYY
        mImgDrawable = binding.imageView.drawable

        mBmp = (mImgDrawable as BitmapDrawable).bitmap
        val options = BitmapFactory.Options()
        // Make sure it is 24 bit color as our image processing algorithm
        // expects this format
        // Make sure it is 24 bit color as our image processing algorithm
        // expects this format
        options.inPreferredConfig = Bitmap.Config.ARGB_8888
        mBmp = BitmapFactory.decodeResource(this.resources, R.mipmap.test, options)

        val mBMP = mBmp
        if (mBMP != null) {
            width = mBMP.width
        }
        if (mBMP != null) {
            height = mBMP.height
        }

        convertImageToGray()
        convertImageToSepia()
        invertImage()
        if (mBmp != null) {
            binding.imageView.setImageBitmap(mBmp)
        }

        binding.label.text = "Brightness = "+ 0.0f



        //FOR BRIGHTNESS====================

        binding.seekBar.setOnSeekBarChangeListener(object : OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                brightness = (progress / 10.0f)
                binding.label.text = "Brightness = $brightness"

                adjustBrightness()
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {}
            override fun onStopTrackingTouch(seekBar: SeekBar) {}
        })



        //OPENCV
        val bmp = BitmapFactory.decodeResource(resources, R.mipmap.test)


        val convertedBmp = convertBmp2Gray(bmp, 619, 619,true)
        Log.e(ContentValues.TAG, "onCreate: $convertedBmp", )
    }


    fun convertBmp2Gray(bitmap: Bitmap, width: Int, height: Int, rotation: Boolean): Bitmap {
        val mat = Mat(
            bitmap.width, bitmap.height,
            CvType.CV_8UC1
        )
        Utils.bitmapToMat(bitmap, mat)
        val outMat = Mat(height, width, CvType.CV_8SC1)
        ImgToGray(mat.nativeObjAddr, outMat.nativeObjAddr)
        Utils.matToBitmap(outMat, bitmap)
        return bitmap
    }




    private fun adjustBrightness() {
        val bitmap = original!!.copy(Bitmap.Config.ARGB_8888, true)
        if(brightness != 0.0f)
        {
            brightness(bitmap, brightness)
        }
        binding.imageView.setImageBitmap(bitmap)
    }

    private fun convertImageToGray() {
        val bitmapWip = Bitmap.createBitmap(width, height, Bitmap.Config.ALPHA_8)
        try {
            //mRelLayout.setBackgroundColor(resources.getColor(android.R.color.white))
            convertToGray(mBmp, bitmapWip)
            binding.imageView.setImageBitmap(bitmapWip)
            binding.imgGrey.setImageBitmap(bitmapWip)
        } catch (e: MyException) {
            val handler = Handler()
            handler.post {
                Toast.makeText(this@MainActivity, e.message, Toast.LENGTH_LONG).show()
            }
        }
    }

    private fun invertImage() {
       // mRelLayout!!.setBackgroundColor(resources.getColor(android.R.color.black))
        var bitmapWip: Bitmap? = null
        bitmapWip = mBmp!!.copy(Bitmap.Config.ARGB_8888, true)
        invertImage(bitmapWip)
        binding.imageView.setImageBitmap(bitmapWip)
        binding.imgInvert.setImageBitmap(bitmapWip)
    }

    private fun convertImageToSepia() {
        //mRelLayout!!.setBackgroundColor(resources.getColor(android.R.color.black))
        try {
            val bitmapWipOther = Bitmap.createBitmap(
                width, height,
                mBmp!!.config
            )
            convertToSepia(mBmp, bitmapWipOther)
            binding.imageView.setImageBitmap(bitmapWipOther)
            binding.imgSepia.setImageBitmap(bitmapWipOther)
        } catch (e: MyException) {
            val handler = Handler()
            handler.post {
                Toast.makeText(this@MainActivity, e.message, Toast.LENGTH_LONG).show()
            }
        }
    }


    private fun warmifyImage() {
        //mRelLayout!!.setBackgroundColor(resources.getColor(android.R.color.black))
        try {
            val bitmapWipOther = Bitmap.createBitmap(
                width, height,
                mBmp!!.config
            )
            warmifyImage(mBmp, bitmapWipOther)
            binding.imageView.setImageBitmap(bitmapWipOther)
        } catch (e: MyException) {
            val handler = Handler()
            handler.post {
                Toast.makeText(this@MainActivity, e.message, Toast.LENGTH_LONG).show()
            }
        }
    }

//    private fun convertImageToBlue() {
//        mRelLayout!!.setBackgroundColor(resources.getColor(android.R.color.black))
//        val bitmapWip = Bitmap.createBitmap(width, height, mBmp!!.config)
//        convertToBlue(mBmp, bitmapWip)
//        mImgView.setImageBitmap(bitmapWip)
//    }

    /**
     * Converts image to red scale
     */
    private fun convertImageToRed() {
        //mRelLayout!!.setBackgroundColor(resources.getColor(android.R.color.black))
        try {
            val bitmapWipOther = Bitmap.createBitmap(
                width, height,
                mBmp!!.config
            )
            convertToRed(mBmp, bitmapWipOther)
            binding.imageView.setImageBitmap(bitmapWipOther)
        } catch (e: MyException) {
            val handler = Handler()
            handler.post {
                Toast.makeText(this@MainActivity, e.message, Toast.LENGTH_LONG).show()
            }
        }
    }


}