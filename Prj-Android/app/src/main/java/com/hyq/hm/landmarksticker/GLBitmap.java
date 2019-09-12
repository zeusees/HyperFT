package com.hyq.hm.landmarksticker;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.opengl.Matrix;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/**
 * Created by 海米 on 2018/10/25.
 */

public class GLBitmap {

    private int aPositionHandle;
    private int uTextureSamplerHandle;
    private int aTextureCoordHandle;
    private int programId;
    private int[] textures;
    private FloatBuffer vertexBuffer;

    private final float[] modelMatrix = new float[16];
    private int uMatrixHandle;

    private final float[] vertexData = {
            1f, -1f,
            -1f, -1f,
            1f, 1f,
            -1f, 1f
    };

    private FloatBuffer textureVertexBuffer;
    private final float[] textureVertexData = {
            1f, 0f,//右下
            0f, 0f,//左下
            1f, 1f,//右上
            0f, 1f//左上
    };
    private Bitmap bitmap;
    public GLBitmap(Context context, int id){
        vertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(vertexData);
        vertexBuffer.position(0);

        textureVertexBuffer = ByteBuffer.allocateDirect(textureVertexData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(textureVertexData);
        textureVertexBuffer.position(0);
        bitmap = BitmapFactory.decodeResource(context.getResources(),id);

    }
    private String vertexShader = "attribute vec4 aPosition;\n" +
            "attribute vec2 aTexCoord;\n" +
            "varying vec2 vTexCoord;\n" +
            "uniform mat4 uMatrix;\n"+
            "void main() {\n" +
            "    vTexCoord=aTexCoord;\n" +
            "    gl_Position = uMatrix*aPosition;\n" +
            "}";
    private String fragmentShader = "varying highp vec2 vTexCoord;\n" +
            "uniform highp sampler2D sTexture;\n"+
            "void main() {\n" +
            "    gl_FragColor = texture2D(sTexture,vec2(vTexCoord.x,1.0 - vTexCoord.y));\n" +
            "}";
    public void initFrame(){
        programId = ShaderUtils.createProgram(vertexShader, fragmentShader);
        aPositionHandle = GLES20.glGetAttribLocation(programId, "aPosition");
        uTextureSamplerHandle=GLES20.glGetUniformLocation(programId,"sTexture");
        aTextureCoordHandle=GLES20.glGetAttribLocation(programId,"aTexCoord");
        uMatrixHandle=GLES20.glGetUniformLocation(programId,"uMatrix");
        textures = new int[1];
        GLES20.glGenTextures(1,textures,0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,textures[0]);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_MIN_FILTER,GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D,0,GLES20.GL_RGBA,bitmap,0);

    }

    public void setPoints(float[] points){
        vertexBuffer.rewind();
        vertexBuffer.put(points);
        vertexBuffer.position(0);
    }

    public void drawFrame(int x,int y,int width,int height,float roll,float yaw,float pitch,boolean isCenter){

        float s = width*8.0f/bitmap.getWidth()/3;
        int w = (int) (bitmap.getWidth()*s);
        int h = (int) (bitmap.getHeight()*s);
        if(isCenter){
            GLES20.glViewport(x - w/2,y - h/2, w, h);
        }else{
            GLES20.glViewport(x - w/2,y - 10, w, h);
        }

        GLES20.glUseProgram(programId);
        GLES20.glEnableVertexAttribArray(aPositionHandle);
        GLES20.glVertexAttribPointer(aPositionHandle, 2, GLES20.GL_FLOAT, false,
                8, vertexBuffer);
        GLES20.glEnableVertexAttribArray(aTextureCoordHandle);
        GLES20.glVertexAttribPointer(aTextureCoordHandle,2,GLES20.GL_FLOAT,false,8,textureVertexBuffer);
        Matrix.setRotateM(modelMatrix,0,roll*1.3f,0,0,-1);
        Matrix.rotateM(modelMatrix,0,yaw*2,0,1,0);
        Matrix.rotateM(modelMatrix,0,pitch*3,-1,0,0);
        GLES20.glUniformMatrix4fv(uMatrixHandle,1,false,modelMatrix,0);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,textures[0]);
        GLES20.glUniform1i(uTextureSamplerHandle,0);
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,0);
        GLES20.glUseProgram(0);
    }
    public void release(){
        GLES20.glDeleteTextures(textures.length,textures,0);
        GLES20.glDeleteProgram(programId);
    }
}
