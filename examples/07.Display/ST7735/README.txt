If you want to use your own image, use an image editing tool
and crop your image to no larger than 160 pixels high and 128 pixels wide.
Save it as a 24-bit color BMP file even if it was originally a 16-bit
color image - because of the way BMPs are stored and displayed!

>convert -depth 24 -resize 160x128 
>convert  -type truecolor -depth 24 -compress None -resize 128x160 pict1.png BMP3:pict2.bmp
