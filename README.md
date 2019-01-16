# CSCI 576 Spring 2016 Term Project 
## Egocentric Videos 

 
### Authors: Roeil Jacob, Timothy Fong 

# Video/Audio Player - GUI Design
![alt text](https://github.com/rjacob/ProjectCSCI576/blob/master/img/gui.png)

# Video/Audio Player - Algorithm
Multi Threaded
 - Decoupling of “Disk Read” and “Display Buffer Writer”
Buffered 
 - Timer discrepancies
   -- Overflows or Underflows
Audio Sync
 - Drift due to playback timer discrepancies

# Video Summarization
 - Uses X Squared algorithm to generate distance based data.
𝑑(𝑖,𝑗)=(𝐹𝑟𝑎𝑚𝑒_𝑝𝑟𝑒𝑣−𝐹𝑟𝑎𝑚𝑒_𝑐𝑢𝑟𝑟 )^2/(𝐹𝑟𝑎𝑚𝑒_𝑐𝑢𝑟𝑟 )
 - Distance threshold based on average + standard deviation calculation of data
 - Better results compared to Euclidean distance and color histogram. 
 - Tried using entropy based metric, was not very robust.  

# Video Indexing
 - Using Computer Vision (CV) techniques
   - Resize Source image using Bilinear Interpolation
   - Convert Source Frame and Video Frames to Grayscale (single channel)
   - Detect Features using Speeded Up Robust Features (SURF 64)
     - Several times faster than SIFT
     - Scale Invariant (Loss in Interpolation)
     - Rotation Invariant (Examples were fixed)
   - Extract Descriptors (both Source and Video Frame)
   - Brute Force Match Descriptors (between Source and Video Frame)
   - Remove Outliers
     - Ignore Features that have moved more than distance √2

## Results 
On the right, the target. On the left, the match.

Alin_Day1_002\11475.png
Frame: 636 (15 matches)
![alt text](https://github.com/rjacob/ProjectCSCI576/blob/master/img/11475.png)

Alin_Day1_002\12651.png
Frame: 1731 (9 matches)
![alt text](https://github.com/rjacob/ProjectCSCI576/blob/master/img/12651.png)

Alin_Day1_002\16192.png
Frame: 3306 (34 matches)
![alt text](https://github.com/rjacob/ProjectCSCI576/blob/master/img/16192.png)

Alin_Day1_002\16700.png
Frame: 3801 (7 matches)
![alt text](https://github.com/rjacob/ProjectCSCI576/blob/master/img/16700.png)

Alin_Day1_002\16954.png
Frame: 4053 (15 matches)
![alt text](https://github.com/rjacob/ProjectCSCI576/blob/master/16954.png)

# Video Corrections
 - Motion Stabilization
 - Using Computer Vision (CV) techniques (OpenCV 2.4.12)
   - Convert Source Frame and Video Frames to Grayscale
   - Detect Features using Speeded up robust features (SURF)
     - Several times faster than SIFT
     - Scale Invariant (Loss in Interpolation)
     - Rotation Invariant
     - Mask Using Region Of Interest (ROI) 50%
   - Extract Descriptors (both Source and Video Frame)
   - Brute Force Match Descriptors (between Source and Video Frame)
   - Remove Outliers
     - Ignore Features that have moved more than √162  9px
   - Consider First 30 (arbitrary) Good Features
   - Compute Homography (Quad Projective Transformation)
   - 2D Transform Image 

# Areas for Improvement
 - Audio Video Player
   - Attempts were made to analyse audio to out-lie regions where there were no human voice (imperceptible to viewer). 
   - Rendering Timer unreliable (use of deterministic clock instead to guarantee 15 Hz)
 - Summarization
   - Consolidate analysis to determine inliers.
     - X-squared, Color Histogram, Entropy, Euclidean Distance
   - Segment image for higher key point resolution.
 - Reference
   - Speed up computation.
   - Memory leaks produced from usage of CV libraries. (SURF is not free!)
 - Stabilization
   - Extract Head-Motion vs general object tracking (Improve outlier methods)
   - To reduce additional outliers focus ROI to only the 4 corners
 - General Bugs
   - Bugs related to Integration

# References
 - Havaldar, P., Medioni, G.: Multimedia Systems, Algorithms, Standards, and Industry Practices
 - Bay H., Tuytelaars T., Van Gool, L.: SURF: Speeded Up Robust Features
