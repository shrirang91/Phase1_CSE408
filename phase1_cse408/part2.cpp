#include <fstream>
#include <iostream>
#include <algorithm>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

typedef struct ColorMap {
    string space;
    int count;
    
    vector<float> first;
    vector<float> second;
    vector<float> third;
} ColorMap;

ColorMap readColorMap(string mapname) {
    ifstream f(mapname.c_str());
    string line;
    
    char *space = (char *)malloc(128);
    
    ColorMap map;
    map.count = 0;
    
    while (getline(f, line)) {
        float c1, c2, c3;
        sscanf(line.c_str(), "%[^(](%f,%f,%f)", space, &c1, &c2, &c3);
                
        map.count += 1;
        map.first.push_back(c1);
        map.second.push_back(c2);
        map.third.push_back(c3);
    }
    
    map.space = space;
    return map;
}

Mat applyMap(Mat scaled, ColorMap map) {
    // Create a 3-channel image the same size as the scaled difference
    Mat applied = Mat(scaled.size(), CV_8UC3);
    
    for (int y = 0; y < scaled.rows; y++) {
        for (int x = 0; x < scaled.cols; x++) {
            Point pos(x, y);
            
            // Obtain the color at the x-y position.
            float value = scaled.at<float>(pos);
            Vec3b &cmapPixel = applied.at<Vec3b>(pos);
            
            // Obtain the partition in the color map
            int partition = int(map.count * (value + 1) / 2.0);
                        
            // Replace the color (RGB is actually read as BGR in OpenCV, so the order is backwards)
            cmapPixel.val[0] = map.first.at(partition);
            cmapPixel.val[1] = map.second.at(partition);
            cmapPixel.val[2] = map.third.at(partition);
        }
    }
    
    return applied;
}

int main(int argc, const char * argv[]) {
    string path;
    bool running = true;
    
    cout << "Enter the path of the folder containing the videos: " << endl << "> ";
    cin >> path;
    
    while (running) {
        string filename;
        string mapname;
        bool ok;        
        bool visualize = true;
        
        Mat frame1, frame2;
        Mat grey1, grey2;
        Mat diff, scaled;
        
        ColorMap map;
        
        int count, fin1, fin2;
        
        // Obtain the filename the user wants to operate on in this run
        cout << endl << "Enter video file name: ";
        cin >> filename;
        // filename = "1.mp4";
        
        // Open a capture object to the video
        VideoCapture cap(path + "/" + filename);
        if (!cap.isOpened()) {
            cout << endl << "[*] ERROR: Couldn't open the video for processing. Restarting." << endl;
            continue; // Restart processing
        }
        
        count = cap.get(CV_CAP_PROP_FRAME_COUNT);
        count -= 4;
        cout << "[*] Frame count for video (" << filename << ") is: " << count << endl;
                
        // Obtain the first frame to process
        cout << endl << "Enter first frame to process: ";
        cin >> fin1;
        
        if (fin1 < 0 || fin1 > count) {
            cout << endl << "[*] ERROR: Invalid first frame number. Restarting." << endl;
            continue;
        }
        
        // Obtain the second frame to process
        cout << "Enter second frame to process: ";
        cin >> fin2;
        
        if (fin2 < 0 || fin2 > count) {
            cout << endl << "[*] ERROR: Invalid second frame number. Restarting." << endl;
            continue;
        }
        
        // Read the color map file name
        cout << "Enter file name of color map: ";
        cin >> mapname;
        // mapname = "colorMap.txt";
        map = readColorMap(mapname);
        
        cout << endl << "[*] Processed color map with " << map.count << " colors in the " << map.space << " color space" << endl;
        
        // Extract the first frame
        cap.set(CV_CAP_PROP_POS_FRAMES, fin1);
        ok = cap.read(frame1);
        if(!ok){
            cout << endl << "[*] ERROR: Couldn't extract first frame. Restarting." << endl;
            continue;
        }
        
        // Extract the second frame
        cap.set(CV_CAP_PROP_POS_FRAMES, fin2);
        ok = cap.read(frame2);
        if(!ok){
            cout << endl << "[*] ERROR: Couldn't extract second frame. Restarting." << endl;
            continue;
        }
        
        cout << "[*] Extracted frames " << fin1 << " and " << fin2 << " from " << filename << endl;
        
        // Convert images to greyscale
        cvtColor(frame1, grey1, CV_BGR2GRAY);
        cvtColor(frame2, grey2, CV_BGR2GRAY);
        
        cout << "[*] Converted extracted frames to greyscale" << endl;
        
        // Save greyscale images to file
        string grey1path = "./frame" + to_string(fin1) + "_grey_" + filename + ".jpg";
        string grey2path = "./frame" + to_string(fin2) + "_grey_" + filename + ".jpg";
        
        imwrite(grey1path, grey1);
        imwrite(grey2path, grey2);
        
        cout << "[*] Saved greyscale frames to <" << grey1path << "> and <" << grey2path << ">" << endl;
        
        // Compute difference of greyscale frames
        absdiff(grey1, grey2, diff);
        
        // Rescale the greyscale difference
        diff.convertTo(scaled, CV_32F, 2/255.0, -1.0);
        
        // Apply the color map
        Mat applied = applyMap(scaled, map);
        
        // Convert the image based on the color space
        if (map.space.compare("RGB") == 0) {
            cvtColor(applied, applied, CV_RGB2BGR);
        }
        else if (map.space.compare("XYZ") == 0) {
            cvtColor(applied, applied, CV_XYZ2BGR);
        }
        else if (map.space.compare("Lab") == 0) {
            cvtColor(applied, applied, CV_Lab2BGR);
        }
        else if (map.space.compare("YUV") == 0) {
            cvtColor(applied, applied, CV_YUV2BGR);
        }
        else if (map.space.compare("YCbCr") == 0) {
            Mat channels[3];
            Mat swapped;
            vector<Mat> merging;
            
            split(applied, channels);
            merging.push_back(channels[0]); // Y
            merging.push_back(channels[2]); // Cr
            merging.push_back(channels[1]); // Cb
            merge(merging, swapped);
            
            cvtColor(applied, applied, CV_YCrCb2BGR);
        }
        else if (map.space.compare("YIQ") == 0) {
            for (int y = 0; y < applied.rows; y++) {
                for (int x = 0; x < applied.cols; x++) {
                    Point pos(x, y);
                    
                    // Obtain the color at the x-y position.
                    Vec3b &pixel = applied.at<Vec3b>(pos);
                
                    float y = pixel.val[0] / 255.0;
                    float i = (pixel.val[1] - 128) / 255.0;
                    float q = (pixel.val[2] - 128) / 255.0;
                    
                    // Replace the YIQ color with BGR color
                    pixel.val[0] = (y - 1.106*i + 1.703*q) * 255;
                    pixel.val[1] = (y - 0.272*i - 0.647*q) * 255;
                    pixel.val[2] = (y + 0.956*i + 0.621*q) * 255;
                }
            }
        }
        else if (map.space.compare("HSL") == 0) {
            Mat channels[3];
            Mat swapped;
            vector<Mat> merging;
            
            split(applied, channels);
            merging.push_back(channels[0]); // H
            merging.push_back(channels[2]); // L
            merging.push_back(channels[1]); // S
            merge(merging, swapped);
            
            cvtColor(swapped, applied, CV_HLS2BGR);
        }
        
        // Allow the user to view the images
        while (visualize) {
            int choice;
            
            cout << endl << "Select which action to take: " << endl;
            cout << "    1. View frame " << fin1 << endl;
            cout << "    2. View frame " << fin2 << endl;
            cout << "    3. View greyscale difference" << endl;
            cout << "    4. View color mapped difference" << endl;
            cout << "    5. Process another video" << endl;
            cout << "    6. Exit" << endl;
            cout << "Enter choice: ";
            cin >> choice;
            
            if (choice == 1) {
                imshow("Frame " + to_string(fin1), frame1);
                waitKey(0);
            }
            else if (choice == 2) {
                imshow("Frame " + to_string(fin2), frame2);
                waitKey(0);
            }
            else if (choice == 3) {
                imshow("Greyscale", diff);
                waitKey(0);
            }
            else if (choice == 4) {
                imshow("Color Mapped", applied);
                waitKey(0);
            }
            else if (choice == 5) {
                visualize = false;
            }
            else {
                return 0;
            }
        }
    }
    
    return 0;
}
