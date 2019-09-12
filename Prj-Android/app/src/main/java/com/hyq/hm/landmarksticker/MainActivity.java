package com.hyq.hm.landmarksticker;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Rect;
import android.hardware.Camera;
import android.opengl.GLES20;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.PermissionChecker;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.CheckBox;
import android.widget.SeekBar;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import trackingsoft.tracking.Face;
import trackingsoft.tracking.FaceTracking;

public class MainActivity extends AppCompatActivity {

    private String modelPath = Environment.getExternalStorageDirectory()
            + File.separator + "FaceTracking";
    public void copyFilesFromAssets(Context context, String oldPath, String newPath) {
        try {
            String[] fileNames = context.getAssets().list(oldPath);
            if (fileNames.length > 0) {
                // directory
                File file = new File(newPath);
                if (!file.mkdir()) {
                    Log.d("mkdir", "can't make folder");

                }

                for (String fileName : fileNames) {
                    copyFilesFromAssets(context, oldPath + "/" + fileName,
                            newPath + "/" + fileName);
                }
            } else {
                // file
                InputStream is = context.getAssets().open(oldPath);
                FileOutputStream fos = new FileOutputStream(new File(newPath));
                byte[] buffer = new byte[1024];
                int byteCount;
                while ((byteCount = is.read(buffer)) != -1) {
                    fos.write(buffer, 0, byteCount);
                }
                fos.flush();
                is.close();
                fos.close();
            }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    void InitModelFiles() {

        String assetPath = "FaceTracking";
//        String sdcardPath = modelPath;

//        Log.e("TAG","sdcardPath=====>"+sdcardPath);
        copyFilesFromAssets(this, assetPath, modelPath);
    }


    private String[] denied;
    private String[] permissions = {Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA};

    public static FaceTracking mMultiTrack106 = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            ArrayList<String> list = new ArrayList<>();
            for (int i = 0; i < permissions.length; i++) {
                if (PermissionChecker.checkSelfPermission(this, permissions[i]) == PackageManager.PERMISSION_DENIED) {
                    list.add(permissions[i]);
                }
            }
            if (list.size() != 0) {
                denied = new String[list.size()];
                for (int i = 0; i < list.size(); i++) {
                    denied[i] = list.get(i);
                }
                ActivityCompat.requestPermissions(this, denied, 5);
            } else {
                init();
            }
        } else {
            init();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == 5) {
            boolean isDenied = false;
            for (int i = 0; i < denied.length; i++) {
                String permission = denied[i];
                for (int j = 0; j < permissions.length; j++) {
                    if (permissions[j].equals(permission)) {
                        if (grantResults[j] != PackageManager.PERMISSION_GRANTED) {
                            isDenied = true;
                            break;
                        }
                    }
                }
            }
            if (isDenied) {
                Toast.makeText(this, "Please give permission.", Toast.LENGTH_SHORT).show();
            } else {
                init();
            }
        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }

    private HandlerThread mHandlerThread;
    private Handler mHandler;
    private final byte[] mNv21Data = new byte[CameraOverlap.PREVIEW_WIDTH * CameraOverlap.PREVIEW_HEIGHT * 2];
    private CameraOverlap cameraOverlap;

    private SurfaceView mSurfaceView;

    private EGLUtils mEglUtils;
    private GLFramebuffer mFramebuffer;
    private GLFrame mFrame;

    private SeekBar seekBarA;
    private SeekBar seekBarB;
    private SeekBar seekBarC;
    private CheckBox checkBox;

    private final int maxFace = 10;
    private final int bitmapCount = 3;
    private List<GLPoints> mPoints;
    private List<GLBitmap> mBitmaps;
    private int index = 0;

    private void init() {
        InitModelFiles();
        cameraOverlap = new CameraOverlap(this);
        mFramebuffer = new GLFramebuffer();
        mFrame = new GLFrame();
        mPoints = new ArrayList<>();
        mBitmaps = new ArrayList<>();

        for (int i = 0; i < maxFace; i++) {
            mPoints.add(new GLPoints());
        }
        for (int i = 0; i < maxFace * bitmapCount; i++) {
            mBitmaps.add(new GLBitmap(this, R.drawable.face_005));
        }
        mHandlerThread = new HandlerThread("DrawFacePointsThread");
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper());

        Log.e("TAG","=========");
        cameraOverlap.setPreviewCallback(new Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {
//                Log.e("TAG","======onPreviewFrame========"+index);
                index++;

                synchronized (mNv21Data) {
                    System.arraycopy(data, 0, mNv21Data, 0, data.length);
                }
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
//                if (mEglUtils == null) {
//                    return;
//                }

                        mFrame.setS(seekBarA.getProgress() / 100.0f);
                        mFrame.setH(seekBarB.getProgress() / 360.0f);
                        mFrame.setL(seekBarC.getProgress() / 100.0f - 1);
                        if (mMultiTrack106 == null) {
                            mMultiTrack106 = new FaceTracking(modelPath + File.separator + "models");
                            mMultiTrack106.FaceTrackingInit(mNv21Data, CameraOverlap.PREVIEW_HEIGHT, CameraOverlap.PREVIEW_WIDTH);
                        } else {
                            mMultiTrack106.Update(mNv21Data, CameraOverlap.PREVIEW_HEIGHT, CameraOverlap.PREVIEW_WIDTH);
//                    Log.e("TAG", "=====Update=====");
                        }
                        boolean rotate270 = cameraOverlap.getOrientation() == 270;


                        List<Face> faceActions = mMultiTrack106.getTrackingInfo();
                        GLES20.glEnable(GLES20.GL_BLEND);
                        GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);
                        mFrame.drawFrame(mFramebuffer.drawFrameBuffer(), mFramebuffer.getMatrix());

                        if (mEglUtils == null) {
//                    Log.e("TAG", "====mEglUtils == null=====");

                            return;
                        }
                        float sw = mEglUtils.getWidth() * 1.0f / CameraOverlap.PREVIEW_HEIGHT;
                        float sh = mEglUtils.getHeight() * 1.0f / CameraOverlap.PREVIEW_WIDTH;
                        int face = 0;
                        int bitmap = 0;
                        int t = 0;
                        int j = 0;

//                        Log.e("TAG", "faceActions length   == " + faceActions.size());
                        for (Face r : faceActions) {
                            float[] points = new float[106 * 2];
                            float[] points_rect = new float[4 * 2];

                            Rect rect = new Rect(r.left, r.top, r.right, r.bottom);

                            float h = rect.top - rect.bottom;
                            float w = rect.right - rect.left;

                            float padd= 0.03f;

                            //r.top += h *padd*5;

                            r.bottom -= h*padd;

                            r.right += w * padd;
                            r.left -= w * padd;


                            int x1 = r.left;
                            int y1 = r.top;
                            //y1 = CameraOverlap.PREVIEW_HEIGHT - y1;

                            int x2 = r.right;
                            int y2 = r.top;
                            //y2 = CameraOverlap.PREVIEW_HEIGHT - y2;

                            int x3 = r.right;
                            int y3 = r.bottom;
                            //y3 = CameraOverlap.PREVIEW_HEIGHT - y3;

                            int x4 = r.left;
                            int y4 = r.bottom;
                            //y4 = CameraOverlap.PREVIEW_HEIGHT - y4;

                            points_rect[0] = view2openglX(x1, CameraOverlap.PREVIEW_HEIGHT);
                            points_rect[1] = view2openglY(y1, CameraOverlap.PREVIEW_WIDTH);

                            points_rect[2] = view2openglX(x2, CameraOverlap.PREVIEW_HEIGHT);
                            points_rect[3] = view2openglY(y2, CameraOverlap.PREVIEW_WIDTH);

                            points_rect[4] = view2openglX(x3, CameraOverlap.PREVIEW_HEIGHT);
                            points_rect[5] = view2openglY(y3, CameraOverlap.PREVIEW_WIDTH);

                            points_rect[6] = view2openglX(x4, CameraOverlap.PREVIEW_HEIGHT);
                            points_rect[7] = view2openglY(y4, CameraOverlap.PREVIEW_WIDTH);


//                            for (int i = 0; i < 106; i++) {
//                              int x;
//                              if (rotate270) {
//                                  x = r.landmarks[i * 2];
//                              } else {
//                                  x = CameraOverlap.PREVIEW_HEIGHT - r.landmarks[i * 2];
//                              }
//                              int y = r.landmarks[i * 2 + 1];
//                              if (t > y || t == 0) {
//                                  t = y;
//                                  j = i;
//                              }
//                              points[i * 2] = view2openglX(x, CameraOverlap.PREVIEW_HEIGHT);
//                              points[i * 2 + 1] = view2openglY(y, CameraOverlap.PREVIEW_WIDTH);
//                              if (i == 69) {
//                                  float[] p = new float[8];
//                                  p[0] = view2openglX(x + 20, CameraOverlap.PREVIEW_HEIGHT);
//                                  p[1] = view2openglY(y - 20, CameraOverlap.PREVIEW_WIDTH);
//                                  p[2] = view2openglX(x - 20, CameraOverlap.PREVIEW_HEIGHT);
//                                  p[3] = view2openglY(y - 20, CameraOverlap.PREVIEW_WIDTH);
//                                  p[4] = view2openglX(x + 20, CameraOverlap.PREVIEW_HEIGHT);
//                                  p[5] = view2openglY(y + 20, CameraOverlap.PREVIEW_WIDTH);
//                                  p[6] = view2openglX(x - 20, CameraOverlap.PREVIEW_HEIGHT);
//                                  p[7] = view2openglY(y + 20, CameraOverlap.PREVIEW_WIDTH);
//                                  mBitmaps.get(bitmap).setPoints(p);
//                                  mBitmaps.get(bitmap).drawFrame((int) (x * sw), (int) (mEglUtils.getHeight() - y * sh), rect.width(), rect.height(), r.roll, r.yaw, r.pitch, i == 69);
//                                  bitmap++;
//                              }
//                         }

//                            int[] arr1 = {50, 72, 69, 45, 105};
  //                          for (int i = 0; i < arr1.length; i++)
  //                            float x = points[i * 2];
 //                               float y = points[i * 2 + 1];
 //                               points[i * 2] = points[arr1[i] * 2];
 //                               points[i * 2 + 1] = points[arr1[i] * 2 + 1];
 //                               points[arr1[i] * 2] = x;
//                                points[arr1[i] * 2 + 1] = y;
//                           }

                            if (checkBox.isChecked()) {
                                //mPoints.get(face).setPoints(points);
                                //mPoints.get(face).drawPoints(mFrame.getRect(),r.ID);
                                mPoints.get(face).setRects(points_rect);
                                mPoints.get(face).drawRects(mFrame.getRect(),r.ID);
                            }
                            face++;
                            if (face == maxFace) {
                                break;
                            }
                        }
                        Log.d("===========", "j = " + j);
                        GLES20.glDisable(GLES20.GL_BLEND);
                        mEglUtils.swap();

                    }
                });
            }
        });
        mSurfaceView = findViewById(R.id.surface_view);
        mSurfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(final SurfaceHolder holder) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mEglUtils != null) {
                            mEglUtils.release();
                        }
                        mEglUtils = new EGLUtils();
                        mEglUtils.initEGL(holder.getSurface());
                        mFramebuffer.initFramebuffer();
                        mFrame.initFrame();
                        mFrame.setSize(mEglUtils.getWidth(), mEglUtils.getHeight(), CameraOverlap.PREVIEW_HEIGHT, CameraOverlap.PREVIEW_WIDTH);
                        for (int i = 0; i < maxFace; i++) {
                            mPoints.get(i).initPoints();
                        }
                        for (int i = 0; i < maxFace * bitmapCount; i++) {
                            mBitmaps.get(i).initFrame();
                        }
                        cameraOverlap.openCamera(mFramebuffer.getSurfaceTexture());
                    }
                });
            }

            @Override
            public void surfaceChanged(final SurfaceHolder holder, int format, final int width, final int height) {


            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
//                mHandler.post(new Runnable() {
//                    @Override
//                    public void run() {
//                        cameraOverlap.release();
//                        mFramebuffer.release();
//                        mFrame.release();
//                        for (int i = 0; i < maxFace;i++){
//                            mPoints.get(i).release();
//                        }
//                        for (int i = 0; i < maxFace* bitmapCount;i++){
//                            mBitmaps.get(i).release();
//                        }
//                        if(mEglUtils != null){
//                            mEglUtils.release();
//                            mEglUtils = null;
//                        }
//                    }
//                });
            }
        });
        if (mSurfaceView.getHolder().getSurface() != null && mSurfaceView.getWidth() > 0) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mEglUtils != null) {
                        mEglUtils.release();
                    }
                    mEglUtils = new EGLUtils();
                    mEglUtils.initEGL(mSurfaceView.getHolder().getSurface());
                    mFramebuffer.initFramebuffer();
                    mFrame.initFrame();
                    mFrame.setSize(mSurfaceView.getWidth(), mSurfaceView.getHeight(), CameraOverlap.PREVIEW_HEIGHT, CameraOverlap.PREVIEW_WIDTH);
                    for (int i = 0; i < maxFace; i++) {
                        mPoints.get(i).initPoints();
                    }
                    for (int i = 0; i < maxFace * bitmapCount; i++) {
                        mBitmaps.get(i).initFrame();
                    }
                    cameraOverlap.openCamera(mFramebuffer.getSurfaceTexture());

//                    cameraOverlap.openCamera(mSurfaceView.getHolder());
                }
            });
        }
        seekBarA = findViewById(R.id.seek_bar_a);
        seekBarB = findViewById(R.id.seek_bar_b);
        seekBarC = findViewById(R.id.seek_bar_c);
        checkBox = findViewById(R.id.checkbox_x);


    }

    private float view2openglX(int x, int width) {
        float centerX = width / 2.0f;
        float t = x - centerX;
        return t / centerX;
    }

    private float view2openglY(int y, int height) {
        float centerY = height / 2.0f;
        float s = centerY - y;
        return s / centerY;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
//        mHandler.post(new Runnable() {
//            @Override
//            public void run() {
//                if(mMultiTrack106 != null){
//                    mMultiTrack106.release();
//                    mMultiTrack106 = null;
//                }
//                mHandlerThread.quit();
//            }
//        });


        cameraOverlap.release();

//        if (mMultiTrack106 != null) {
//            mMultiTrack106.release();
//            mMultiTrack106 = null;
//        }

        cameraOverlap.release();
        mFramebuffer.release();
        mFrame.release();
        for (int i = 0; i < maxFace;i++){
            mPoints.get(i).release();
        }
        for (int i = 0; i < maxFace* bitmapCount;i++){
            mBitmaps.get(i).release();
        }
        if(mEglUtils != null){
            mEglUtils.release();
            mEglUtils = null;
        }
        mHandlerThread.quit();

        mHandler.removeCallbacks(null);

    }
}
