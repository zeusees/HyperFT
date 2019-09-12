package com.hyq.hm.landmarksticker;

import android.graphics.Rect;
import android.opengl.GLES20;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.Random;

/**
 * Created by 海米 on 2018/11/29.
 */

public class GLPoints {
    private FloatBuffer vertexBuffer;
    private int bufferLength = 106*2*10;

    private FloatBuffer vertexBuffer_rect;

    private int bufferLength_rect= 4*2*10;

    private int programId = -1;
    private int aPositionHandle;

    private int[] vertexBuffers;

//70,130,180
    private String fragmentShader_template =
            "void main() {\n" +
            "    gl_FragColor = vec4(%f,%f,%f,1.0);\n" +
            "}";

    private String fragmentShader;
    private  String vertexShader = "attribute vec2 aPosition;\n" +
            "void main() {\n" +
            "    gl_Position = vec4(aPosition,0.0,1.0);\n" +
            "    gl_PointSize = 8.0;\n"+

            "}";

    public GLPoints(){
        vertexBuffer = ByteBuffer.allocateDirect(bufferLength)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        vertexBuffer.position(0);

        vertexBuffer_rect= ByteBuffer.allocateDirect(bufferLength_rect)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        vertexBuffer_rect.position(0);
        Random random = new Random(System.currentTimeMillis());
        fragmentShader=  String.format(fragmentShader_template, random.nextFloat(),random.nextFloat(),random.nextFloat());


    }

    public void initPoints(){


        programId = ShaderUtils.createProgram(vertexShader,fragmentShader);

        aPositionHandle = GLES20.glGetAttribLocation(programId, "aPosition");

        vertexBuffers = new int[1];
        GLES20.glGenBuffers(1,vertexBuffers,0);
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, vertexBuffers[0]);

        GLES20.glBufferData(GLES20.GL_ARRAY_BUFFER, bufferLength, vertexBuffer,GLES20.GL_STATIC_DRAW);
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0);

//        GLES20.glBufferData(GLES20.GL_ARRAY_BUFFER, bufferLength_rect, vertexBuffer_rect,GLES20.GL_STATIC_DRAW);
//        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0);

    }
    public void setPoints(float[] points){
        vertexBuffer.rewind();
        vertexBuffer.put(points);
        vertexBuffer.position(0);
    }

    public void setRects(float[] points){
        vertexBuffer_rect.rewind();
        vertexBuffer_rect.put(points);
        vertexBuffer_rect.position(0);
        Log.d("draw rect",String.format("%f %f %f %f",points[0], points[1],points[2],points[3]));


    }

    public void drawPoints(Rect rect,int id){
        //Random random = new Random(id)?1;

        float a[]={0.0f,0.7f,0.8f,0.0f,1.0f,0.3f,0.0f,1.0f,0.3f,1.0f,0.0f,1.0f};
        float r = a[id%8];
        float g = a[id%8+1];
        float b = a[id%8+2];
        fragmentShader =  String.format(fragmentShader_template,r,g,b);

        //long t0 = System.currentTimeMillis();
        programId = ShaderUtils.createProgram(vertexShader,fragmentShader);
        //long t1 = System.currentTimeMillis();
        //Log.d("shader",String.format("shader %d",t1-t0));

        GLES20.glViewport(rect.left, rect.top, rect.right, rect.bottom);
        GLES20.glUseProgram(programId);
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, vertexBuffers[0]);
        GLES20.glBufferSubData(GLES20.GL_ARRAY_BUFFER,0,bufferLength,vertexBuffer);
        GLES20.glEnableVertexAttribArray(aPositionHandle);
        GLES20.glVertexAttribPointer(aPositionHandle, 2, GLES20.GL_FLOAT, false, 0, 0);
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0);
        GLES20.glDrawArrays(GLES20.GL_POINTS, 0, 106);
//        (-0.8f,-0.8f,0.8f,0.8f);
        //  GLES20.glDrawArrays(GLES20.GL_LINE_LOOP, 0,4);
//        GLES20.glre(GLES20.GL_POINTS, 0, 106);
    }

    public void drawRects(Rect rect,int id){
        float a[]={0.0f,0.7f,0.8f,0.0f,1.0f,0.3f,0.0f,1.0f,0.3f,1.0f,0.0f,1.0f};
        float r = a[id%8];
        float g = a[id%8+1];
        float b = a[id%8+2];
        fragmentShader =  String.format(fragmentShader_template,r,g,b);


        //long t0 = System.currentTimeMillis();
        programId = ShaderUtils.createProgram(vertexShader,fragmentShader);
        GLES20.glViewport(rect.left, rect.top, rect.right, rect.bottom);
        GLES20.glUseProgram(programId);
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, vertexBuffers[0]);
        GLES20.glBufferSubData(GLES20.GL_ARRAY_BUFFER,0,bufferLength_rect,vertexBuffer_rect);
        GLES20.glEnableVertexAttribArray(aPositionHandle);
        GLES20.glVertexAttribPointer(aPositionHandle, 2, GLES20.GL_FLOAT, false,
                0, 0);
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0);
        GLES20.glLineWidth(6.0f);
        GLES20.glDrawArrays(GLES20.GL_LINE_LOOP, 0, 4);
    }

    public void release(){
        GLES20.glDeleteProgram(programId);
        GLES20.glDeleteBuffers(1,vertexBuffers,0);
    }
}
