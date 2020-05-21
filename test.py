import cv2

# Load image
img = cv2.imread('syn_2.tif',cv2.IMREAD_UNCHANGED)
print(img[2000][1000])