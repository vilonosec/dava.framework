package com.dava.framework;

import java.util.HashMap;
import java.util.Map;

import android.annotation.SuppressLint;
import android.util.Log;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.FrameLayout;

@SuppressLint("UseSparseArrays")
public class JNIWebView {
	final static String TAG = "JNIWebView";
	static Map<Integer, WebView> views = new HashMap<Integer, WebView>();
	
	private static class InternalViewClient extends WebViewClient {
		int id;
		
		public InternalViewClient(int _id) {
			id = _id;
		}
		
		@Override
		public boolean shouldOverrideUrlLoading(WebView view, String url) {
			int res = OnUrlChange(id, url);
			
			/*enum eAction
			{
				PROCESS_IN_WEBVIEW = 0,
				PROCESS_IN_SYSTEM_BROWSER,
				NO_PROCESS,
				ACTIONS_COUNT
			};*/

			if (res == 0) {
				return false;
			}
			else if (res == 1) {

				final JNIActivity activity = JNIActivity.GetActivity();
				WebView webView = new WebView(activity);
				webView.loadUrl(url);
				return true;
			}
			return true;
		}
	}
	
	public static void Initialize(final int id, final float x, final float y, final float dx, final float dy)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (views.containsKey(id))
				{
					Log.d(TAG, String.format("WebView with id %d already initialized", id));
					return;
				}
				WebView webView = new WebView(activity);
				FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
						new FrameLayout.MarginLayoutParams((int)(dx + 0.5f), (int)(dy + 0.5f)));
				params.leftMargin = (int)x;
				params.topMargin = (int)y;
				params.width = (int)(dx + 0.5f);
				params.height = (int)(dy + 0.5f);
				webView.setWebViewClient(new InternalViewClient(id));
				webView.getSettings().setJavaScriptEnabled(true);
				
				activity.addContentView(webView, params);
				views.put(id, webView);
			}
		});
	}
	
	public static void Deinitialize(final int id)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				WebView view = views.get(id);
				views.remove(id);
				
				((ViewGroup)view.getParent()).removeView(view);
			}
		});
	}
	
	public static void OpenURL(final int id, final String url)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				WebView webView = views.get(id);
				webView.loadUrl(url);
			}
		});
	}
	
	public static void SetRect(final int id, final float x, final float y, final float dx, final float dy)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				
				WebView view = views.get(id);
				FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) view.getLayoutParams();
				params.leftMargin = (int)x;
				params.topMargin = (int)y;
				params.width = (int)(dx + 0.5f);
				params.height = (int)(dy + 0.5f); 
				view.setLayoutParams(params);
			}
		});
	}
	
	public static void SetVisible(final int id, final boolean isVisible)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				WebView view = views.get(id);
				view.setVisibility(isVisible ? WebView.VISIBLE : WebView.INVISIBLE);
			}
		});
	}
	
	private static native int OnUrlChange(int id, String url);
}
