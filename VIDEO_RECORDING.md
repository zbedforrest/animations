# Video Recording Feature

This document explains how to use the video recording functionality in the animation project.

## Overview

The video recording feature allows you to capture the animated recreation view and export it as an MP4 video file using ffmpeg.

## How to Use

1. **Start the Application**
   ```bash
   make run
   ```

2. **Navigate to Recreation View**
   - From the analyzer view, press `S` to go to shader view, then navigate to the recreation view
   - Or use the appropriate navigation keys to reach the recreation view

3. **Start Recording**
   - Press `R` to start recording
   - The animation will automatically start if it's not already running
   - You'll see a red "RECORDING" indicator and frame counter on screen

4. **Stop Recording**
   - Press `R` again to stop recording
   - The system will automatically export frames and attempt to create a video

## Video Output

- **Format**: MP4 (H.264 codec)
- **Framerate**: 30 fps
- **Quality**: High quality (CRF 18)
- **Output file**: `animation_output.mp4` in the project directory
- **Max duration**: ~2 minutes (3000 frames at 30fps)

## Controls

| Key | Action |
|-----|--------|
| `R` | Start/Stop video recording |
| `SPACE` | Play/Pause animation |
| `A` | Return to analyzer view |
| `LEFT`/`RIGHT` | Navigate animation frames manually |

## Requirements

- **ffmpeg**: Must be installed and available in your system PATH
  - Windows: Download from https://ffmpeg.org/download.html
  - Add ffmpeg to your PATH environment variable

## Process Flow

1. When recording starts:
   - Frame buffer is allocated
   - Animation begins (if not already running)
   - Each frame is captured using `LoadImageFromScreen()`

2. When recording stops:
   - Frames are exported as PNG files to `video_frames/` directory
   - ffmpeg is called to convert frames to MP4
   - Temporary frames are cleaned up (if ffmpeg succeeds)

## Troubleshooting

### ffmpeg not found
If you see "ffmpeg command failed", ensure ffmpeg is installed and in your PATH:
```bash
ffmpeg -version
```

### Manual video creation
If automatic video creation fails, you can manually run:
```bash
ffmpeg -y -framerate 30 -i video_frames/frame_%06d.png -c:v libx264 -pix_fmt yuv420p -crf 18 animation_output.mp4
```

### Memory limitations
- Recording is limited to 3000 frames (~2 minutes at 30fps)
- Each frame uses screen resolution worth of memory
- On systems with limited RAM, consider reducing `max_frames` in the code

## Technical Details

- Screen capture resolution: Window size (1000x800 in recreation view)
- Frame storage: In-memory buffer during recording
- Export format: Sequential PNG files
- Video encoding: H.264 with yuv420p pixel format for wide compatibility
