package com.hyq.hm.landmarksticker;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.RelativeLayout;


public class FixedAspectRatioRelativeLayout extends RelativeLayout {
    private int mAspectRatioWidth = 480;
    private int mAspectRatioHeight = 640;

    public FixedAspectRatioRelativeLayout(Context context) {
        super(context);
    }

    public FixedAspectRatioRelativeLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public FixedAspectRatioRelativeLayout(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int originalWidth = MeasureSpec.getSize(widthMeasureSpec);
        int originalHeight = MeasureSpec.getSize(heightMeasureSpec);

        int finalWidth, finalHeight;

        if (originalWidth > originalHeight) {
            finalWidth = originalHeight * mAspectRatioWidth / mAspectRatioHeight;
            finalHeight = originalHeight;
        } else {
            finalWidth = originalWidth;
            finalHeight = originalWidth * mAspectRatioHeight / mAspectRatioWidth;
        }
        super.onMeasure(
                MeasureSpec.makeMeasureSpec(finalWidth, MeasureSpec.EXACTLY),
                MeasureSpec.makeMeasureSpec(finalHeight, MeasureSpec.EXACTLY));
    }
}
