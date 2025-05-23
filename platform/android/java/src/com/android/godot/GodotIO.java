/*************************************************************************/
/*  GodotIO.java                                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
package com.android.godot;
import java.util.HashMap;
import java.util.Locale;
import android.net.Uri;
import android.content.Intent;
import android.content.res.AssetManager;
import java.io.InputStream;
import java.io.IOException;
import android.app.*;
import android.content.*;
import android.view.*;
import android.view.inputmethod.InputMethodManager;
import android.os.*;
import android.util.Log;
import android.graphics.*;
import android.text.method.*;
import android.text.*;
import android.media.*;
import android.hardware.*;
import android.content.*;
import android.content.pm.ActivityInfo;
import com.android.godot.input.*;
//android.os.Build

// Wrapper for native library

public class GodotIO {


	AssetManager am;
	Godot activity;
	GodotEditText edit;

	Context applicationContext;
	MediaPlayer mediaPlayer;

	final int SCREEN_LANDSCAPE=0;
	final int SCREEN_PORTRAIT=1;
	final int SCREEN_REVERSE_LANDSCAPE=2;
	final int SCREEN_REVERSE_PORTRAIT=3;
	final int SCREEN_SENSOR_LANDSCAPE=4;
	final int SCREEN_SENSOR_PORTRAIT=5;
	final int SCREEN_SENSOR=6;

	/////////////////////////
	/// FILES
	/////////////////////////

	public int last_file_id=1;

	class AssetData {


		public boolean eof=false;
		public String path;
		public InputStream is;
		public int len;
		public int pos;
	}


	HashMap<Integer,AssetData> streams;


	public int file_open(String path,boolean write) {

		//System.out.printf("file_open: Attempt to Open %s\n",path);

		if (write)
			return -1;


		AssetData ad = new AssetData();

		try {
			ad.is = am.open(path);

		} catch (Exception e) {

			//System.out.printf("Exception on file_open: %s\n",e);
			return -1;
		}

		try {
			ad.len=ad.is.available();
		} catch (Exception e) {

			System.out.printf("Exception availabling on file_open: %s\n",e);
			return -1;
		}

		ad.path=path;
		ad.pos=0;
		++last_file_id;
		streams.put(last_file_id,ad);

		return last_file_id;
	}
	public int file_get_size(int id) {

		if (!streams.containsKey(id)) {
			System.out.printf("file_get_size: Invalid file id: %d\n",id);
			return -1;
		}

		return streams.get(id).len;

	}
	public void file_seek(int id,int bytes) {

		if (!streams.containsKey(id)) {
			System.out.printf("file_get_size: Invalid file id: %d\n",id);
			return;
		}
		//seek sucks
		AssetData ad = streams.get(id);
		if (bytes>ad.len)
			bytes=ad.len;
		if (bytes<0)
			bytes=0;

		try {

		if (bytes > (int)ad.pos) {
			int todo=bytes-(int)ad.pos;
			while(todo>0) {
				todo-=ad.is.skip(todo);
			}
			ad.pos=bytes;
		} else if (bytes<(int)ad.pos) {

			ad.is=am.open(ad.path);

			ad.pos=bytes;
			int todo=bytes;
			while(todo>0) {
				todo-=ad.is.skip(todo);
			}
		}

		ad.eof=false;
		} catch (IOException e) {

			System.out.printf("Exception on file_seek: %s\n",e);
			return;
		}


	}

	public int file_tell(int id) {

		if (!streams.containsKey(id)) {
			System.out.printf("file_read: Can't tell eof for invalid file id: %d\n",id);
			return 0;
		}

		AssetData ad = streams.get(id);
		return ad.pos;
	}
	public boolean file_eof(int id) {

		if (!streams.containsKey(id)) {
			System.out.printf("file_read: Can't check eof for invalid file id: %d\n",id);
			return false;
		}

		AssetData ad = streams.get(id);
		return ad.eof;
	}

	public byte[] file_read(int id, int bytes) {

		if (!streams.containsKey(id)) {
			System.out.printf("file_read: Can't read invalid file id: %d\n",id);
			return new byte[0];
		}


		AssetData ad = streams.get(id);

		if (ad.pos + bytes > ad.len) {

			bytes=ad.len-ad.pos;
			ad.eof=true;
		}


		if (bytes==0) {

			return new byte[0];
		}



		byte[] buf1=new byte[bytes];
		int r=0;
		try {
			r = ad.is.read(buf1);
		} catch (IOException e) {

			System.out.printf("Exception on file_read: %s\n",e);
			return new byte[bytes];
		}

		if (r==0) {
			return new byte[0];
		}

		ad.pos+=r;

		if (r<bytes) {

			byte[] buf2=new byte[r];
			for(int i=0;i<r;i++)
				buf2[i]=buf1[i];
			return buf2;
		} else {

			return buf1;
		}

	}

	public void file_close(int id) {

		if (!streams.containsKey(id)) {
			System.out.printf("file_close: Can't close invalid file id: %d\n",id);
			return;
		}

		streams.remove(id);

	}


	/////////////////////////
	/// DIRECTORIES
	/////////////////////////


	class AssetDir {

		public String[] files;
		public int current;
	}

	public int last_dir_id=1;

	HashMap<Integer,AssetDir> dirs;

	public int dir_open(String path) {

		AssetDir ad = new AssetDir();
		ad.current=0;

		try {
			ad.files = am.list(path);
		} catch (IOException e) {

			System.out.printf("Exception on dir_open: %s\n",e);
			return -1;
		}

		++last_dir_id;
		dirs.put(last_dir_id,ad);

		return last_dir_id;

	}

	public String dir_next(int id) {

		if (!dirs.containsKey(id)) {
			System.out.printf("dir_next: invalid dir id: %d\n",id);
			return "";
		}

		AssetDir ad = dirs.get(id);
		if (ad.current>=ad.files.length)
			return "";
		String r = ad.files[ad.current];
		ad.current++;
		return r;

	}

	public void dir_close(int id) {

		if (!dirs.containsKey(id)) {
			System.out.printf("dir_close: invalid dir id: %d\n",id);
			return;
		}

		dirs.remove(id);
	}



	GodotIO(Godot p_activity) {

		am=p_activity.getAssets();
		activity=p_activity;
		streams=new HashMap<Integer,AssetData>();
		dirs=new HashMap<Integer,AssetDir>();
		applicationContext = activity.getApplicationContext();

	}


	/////////////////////////
	// AUDIO
	/////////////////////////

	private Object buf;
	private Thread mAudioThread;
	private AudioTrack mAudioTrack;

	public Object audioInit(int sampleRate, int desiredFrames) {
		int channelConfig = AudioFormat.CHANNEL_CONFIGURATION_STEREO;
		int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
		int frameSize = 4;

		System.out.printf("audioInit: initializing audio:\n");

		//Log.v("Fusion", "Godot audio: wanted " + (isStereo ? "stereo" : "mono") + " " + (is16Bit ? "16-bit" : "8-bit") + " " + ((float)sampleRate / 1000f) + "kHz, " + desiredFrames + " frames buffer");

		// Let the user pick a larger buffer if they really want -- but ye
		// gods they probably shouldn't, the minimums are horrifyingly high
		// latency already
		desiredFrames = Math.max(desiredFrames, (AudioTrack.getMinBufferSize(sampleRate, channelConfig, audioFormat) + frameSize - 1) / frameSize);

		mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
			channelConfig, audioFormat, desiredFrames * frameSize, AudioTrack.MODE_STREAM);

		audioStartThread();

		//Log.v("Fusion", "Godot audio: got " + ((mAudioTrack.getChannelCount() >= 2) ? "stereo" : "mono") + " " + ((mAudioTrack.getAudioFormat() == AudioFormat.ENCODING_PCM_16BIT) ? "16-bit" : "8-bit") + " " + ((float)mAudioTrack.getSampleRate() / 1000f) + "kHz, " + desiredFrames + " frames buffer");

		buf = new short[desiredFrames * 2];
		return buf;
	}

	public void audioStartThread() {
		mAudioThread = new Thread(new Runnable() {
		    public void run() {
			mAudioTrack.play();
			GodotLib.audio();
		}
		});

		// I'd take REALTIME if I could get it!
		mAudioThread.setPriority(Thread.MAX_PRIORITY);
		mAudioThread.start();
	}

	public void audioWriteShortBuffer(short[] buffer) {
		for (int i = 0; i < buffer.length; ) {
			int result = mAudioTrack.write(buffer, i, buffer.length - i);
			if (result > 0) {
				i += result;
			} else if (result == 0) {
				try {
				    Thread.sleep(1);
				} catch(InterruptedException e) {
				    // Nom nom
				}
			} else {
				Log.w("Fusion", "Godot audio: error return from write(short)");
				return;
			}
		}
	}



	public void audioQuit() {
		if (mAudioThread != null) {
			try {
				mAudioThread.join();
			} catch(Exception e) {
				Log.v("Fusion", "Problem stopping audio thread: " + e);
			}
			mAudioThread = null;

		//Log.v("Fusion", "Finished waiting for audio thread");
		}

		if (mAudioTrack != null) {
			mAudioTrack.stop();
			mAudioTrack = null;
		}
	}

	public void audioPause(boolean p_pause) {

		if (p_pause)
			mAudioTrack.pause();
		else
			mAudioTrack.play();
	}

	/////////////////////////
	// MISCELANEOUS OS IO
	/////////////////////////



	public int openURI(String p_uri) {

		try {
			Log.v("MyApp", "TRYING TO OPEN URI: " + p_uri);
			String path = p_uri;
			String type="";
			if (path.startsWith("/")) {
				//absolute path to filesystem, prepend file://
				path="file://"+path;
				if (p_uri.endsWith(".png") || p_uri.endsWith(".jpg") || p_uri.endsWith(".gif") || p_uri.endsWith(".webp")) {

					type="image/*";
				}
			}

			Intent intent = new Intent();
			intent.setAction(Intent.ACTION_VIEW);
			if (!type.equals("")) {
				intent.setDataAndType(Uri.parse(path), type);
			} else {
				intent.setData(Uri.parse(path));
			}

			activity.startActivity(intent);
			return 0;
		} catch (ActivityNotFoundException e) {

			return 1;
		}
	}

	public String getDataDir() {

		return activity.getFilesDir().getAbsolutePath();
	}

	public String getLocale() {

		return Locale.getDefault().toString();
	}

	public String getModel() {
		return Build.MODEL;
	}

	public boolean needsReloadHooks() {

		return android.os.Build.VERSION.SDK_INT < 11;
	}

	public void showKeyboard(String p_existing_text) {
		if(edit != null)
			edit.showKeyboard(p_existing_text);

		//InputMethodManager inputMgr = (InputMethodManager)activity.getSystemService(Context.INPUT_METHOD_SERVICE);
		//inputMgr.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
	};

	public void hideKeyboard() {
		if(edit != null)
			edit.hideKeyboard();

        InputMethodManager inputMgr = (InputMethodManager)activity.getSystemService(Context.INPUT_METHOD_SERVICE);
        View v = activity.getCurrentFocus();
        if (v != null) {
            inputMgr.hideSoftInputFromWindow(v.getWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);
        } else {
            inputMgr.hideSoftInputFromWindow(new View(activity).getWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);
        }
	};

	public void setScreenOrientation(int p_orientation) {

		switch(p_orientation) {

			case SCREEN_LANDSCAPE: {
				activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
			} break;
			case SCREEN_PORTRAIT: {
				activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
			} break;
			case SCREEN_REVERSE_LANDSCAPE: {
				activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
			} break;
			case SCREEN_REVERSE_PORTRAIT: {
				activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT);
			} break;
			case SCREEN_SENSOR_LANDSCAPE: {
				activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
			} break;
			case SCREEN_SENSOR_PORTRAIT: {
				activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT);
			} break;
			case SCREEN_SENSOR: {
				activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_FULL_SENSOR);
			} break;

		}
	};
	
	public void setEdit(GodotEditText _edit) {
		edit = _edit;
	}

	public void playVideo(String p_path)
	{
		Uri filePath = Uri.parse(p_path);
		mediaPlayer = new MediaPlayer();

		try {
			mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
			mediaPlayer.setDataSource(applicationContext, filePath);
			mediaPlayer.prepare();
			mediaPlayer.start();
		}
		catch(IOException e)
        {
            System.out.println("IOError while playing video");
        }
	}

	public boolean isVideoPlaying() {
		if (mediaPlayer != null) {
			return mediaPlayer.isPlaying();
		}
		return false;
	}

	public void pauseVideo() {
		if (mediaPlayer != null) {
			mediaPlayer.pause();
		}
	}

	public void stopVideo() {
		if (mediaPlayer != null) {
			mediaPlayer.release();
			mediaPlayer = null;
		}
	}


	public static final int SYSTEM_DIR_DESKTOP=0;
	public static final int SYSTEM_DIR_DCIM=1;
	public static final int SYSTEM_DIR_DOCUMENTS=2;
	public static final int SYSTEM_DIR_DOWNLOADS=3;
	public static final int SYSTEM_DIR_MOVIES=4;
	public static final int SYSTEM_DIR_MUSIC=5;
	public static final int SYSTEM_DIR_PICTURES=6;
	public static final int SYSTEM_DIR_RINGTONES=7;


	public String getSystemDir(int idx) {

		String what="";
		switch(idx) {
			case SYSTEM_DIR_DESKTOP: {
				//what=Environment.DIRECTORY_DOCUMENTS;
				what=Environment.DIRECTORY_DOWNLOADS;
			} break;
			case SYSTEM_DIR_DCIM: {
				what=Environment.DIRECTORY_DCIM;

			} break;
			case SYSTEM_DIR_DOCUMENTS: {
				what=Environment.DIRECTORY_DOWNLOADS;
				//what=Environment.DIRECTORY_DOCUMENTS;
			} break;
			case SYSTEM_DIR_DOWNLOADS: {
				what=Environment.DIRECTORY_DOWNLOADS;

			} break;
			case SYSTEM_DIR_MOVIES: {
				what=Environment.DIRECTORY_MOVIES;

			} break;
			case SYSTEM_DIR_MUSIC: {
				what=Environment.DIRECTORY_MUSIC;
			} break;
			case SYSTEM_DIR_PICTURES: {
				what=Environment.DIRECTORY_PICTURES;
			} break;
			case SYSTEM_DIR_RINGTONES: {
				what=Environment.DIRECTORY_RINGTONES;

			} break;
		}

		if (what.equals(""))
			return "";
		return Environment.getExternalStoragePublicDirectory(what).getAbsolutePath();
	}

	protected static final String PREFS_FILE = "device_id.xml";
	protected static final String PREFS_DEVICE_ID = "device_id";

	public static String unique_id="";
	public String getUniqueID() {

		return  unique_id;
	}

}
