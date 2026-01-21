import cv2
import sys
import ctypes

# Windows Constants
HWND_BOTTOM    = 1
SWP_NOSIZE     = 0x0001
SWP_NOMOVE     = 0x0002
SWP_NOACTIVATE = 0x0010

def play_video(video_path: str) -> None:
    cap         = cv2.VideoCapture(video_path)
    window_name = "outro_final.mp4"
    
    # Create a named window and force it to be 1280x720
    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    cv2.resizeWindow(window_name, 1280, 720)

    # Ensure the window is created in the OS!
    ret, frame = cap.read()
    if not ret: return;
    cv2.imshow(window_name, frame)

    # --- Push Window to Background --- #
    hwnd = ctypes.windll.user32.FindWindowW(None, window_name)
    if hwnd:
        ctypes.windll.user32.SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE)
    
    while cap.isOpened():
        (ret, frame) = cap.read()
        if not ret: break;
            
        cv2.imshow(window_name, frame)
        
        # Play at roughly 30fps
        if cv2.waitKey(33) & 0xFF == ord('q'): break;
            
    cap.release()
    cv2.destroyAllWindows()

    return;

if __name__ == "__main__":
    if len(sys.argv) > 1: play_video(sys.argv[1])
