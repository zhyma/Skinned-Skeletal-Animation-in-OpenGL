Updated at 2018 Feb. 15th, by Zhaoyuan Ma
----
* Removed animation from the original code.
* Centered the Master Chief.
* This code is modified to read *.angle files (under bin32/ for now) from 3D motion capture resources (in this example, those inputs are directly converted from [NTU RGB+D dataset](https://github.com/shahroudy/NTURGB-D)), playback and save as 2D images.
----
/**** The format of .angle file ***/
Number of frames in total
Number of joints (frame No. 0)
joint1 (head) x, y, z
joint2 (left shoulder) x, y, z
joint3 (left elbow) x, y, z
joint4 (left hand) x, y, z
joint5 (right shoulder) x, y, z
joint6 (right elbow) x, y, z
joint7 (right hand) x, y, z
Number of joints (frame No. 1)
......