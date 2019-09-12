package trackingsoft.tracking;

import android.graphics.Rect;

public class Utils {
    static Rect get_boundingbox(int pts[],final int num_pts)
    {
        int  _x1 = pts[0] ;
        int  _y1 = pts[1] ;
        int  _x2 = pts[0] ;
        int  _y2 = pts[1] ;

        for(int i  = 1 ; i < num_pts; i++)
        {
            int x = pts[i*2];
            int y = pts[i*2+1];
            if(x>_x2)
                _x2 = x;
            if(x<_x1)
                _x1 = x;
            if(y>_y2)
                _y2 = y;
            if(y<_y1)
                _y1 = y;
        }

        Rect rect = new Rect(_x1,_y1,_x2,_y2);
        return rect;
    }

}
