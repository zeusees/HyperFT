package trackingsoft.tracking;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

import static java.lang.StrictMath.abs;

/**
 * Created by yujinke on 17/07/2018.
 */



public class FaceTracking {

    static {
        System.loadLibrary("Tracking-lib");
    }

    public native static void update(byte[] data, int height,int width,long session);
    public native static void initTracking(byte[] data, int height,int width,long session);
    public native static long createSession(String modelPath);
    public native static void releaseSession(long session);
    public native static int getTrackingNum(long session);
    public native static int[] getTrackingLandmarkByIndex(int index,long session);
    public native static int[] getTrackingLocationByIndex(int index,long session);
    public native static int[] getAttributeByIndex(int index,long session);
    public native static float[] getEulerAngleByIndex(int index,long session);
    public native static int getTrackingIDByIndex(int index,long session);


    private long session;
    private List<Face> faces;
    private int tracking_seq = 0;


    public FaceTracking(String pathModel)
    {
        session = createSession(pathModel);
        faces = new ArrayList<Face>();

    }
    public void release(){
        releaseSession(session);
    }

    public  void FaceTrackingInit(byte[] data , int height,int width)
    {
        initTracking(data,height,width,session);
    }

    public boolean postProcess(int[] landmark_prev, int[] landmark_curr)
    {
        int  diff = 0 ;

        for(int i = 0 ; i < 2*2; i ++)
        {
            diff+=  abs(landmark_curr[i] - landmark_prev[i]);

        }

        if(  diff < 5.0*2*2)
        {
            Log.d("test","stablizer");

            for(int j=0 ; j< 2*2; j++)
            {
                landmark_curr[j]  = landmark_prev[j];
            }
            return true;
        }
        else if(diff < 8.0*2*2 ){
            for(int j=0 ; j< 2*2; j++)
            {
                landmark_curr[j]  =(landmark_curr[j] +landmark_prev[j])/2;
            }
            return true;
        }
        return false;
    }

    public int find_id_face(List<Face> faces,int targetID )
    {
        for(int i = 0 ; i < faces.size() ; i++)
        {
            if(faces.get(i).ID == targetID)
                return i;
        }
        return -1;
    }

    public void postProcess_aux(int[] landmark_prev, int[] landmark_curr)
    {

        for(int i = 0 ; i < 2*2; i ++)
        {
            landmark_curr[i]  =(landmark_curr[i]);

        }
    }




    public void Update(byte[] data , int height,int width)
    {
        update(data,height,width,session);
        int numsFace = getTrackingNum(session);
        List<Face> _faces = new ArrayList<Face>();
        for(int i = 0 ; i < numsFace ; i++) {
            int ID_GET = -1;
            int flag = -1;
            int[] faceRect = getTrackingLocationByIndex( i,session);
            int id = getTrackingIDByIndex(i,session);
            int[] landmarks = getTrackingLandmarkByIndex( i,session);
            if(tracking_seq>0)
            {
                ID_GET = find_id_face(faces,id);
                if(ID_GET!=-1) {
                    boolean res = postProcess(faces.get(ID_GET).landmarks, landmarks);
                    if(res)
                        flag = -2;
                }
                if(ID_GET!=-1){
                    if(faces.get(ID_GET).isStable)
                    {
                        postProcess_aux(faces.get(ID_GET).landmarks, landmarks);
                    }
                }
            }

            Face face = new Face(landmarks[0], landmarks[1], landmarks[2] - landmarks[0], landmarks[3] - landmarks[1], landmarks,id);

            //Face face = new Face(faceRect[0], faceRect[1], faceRect[2], faceRect[3], landmarks,id);
            if (flag == -2)
                face.isStable = true;
            else
                face.isStable = false;
            _faces.add(face);
        }
        faces.clear();
        faces = _faces;
        tracking_seq+=1;

    }


    public List<Face> getTrackingInfo(){
        return faces;

    }

}
