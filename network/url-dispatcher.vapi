namespace URLDispatcher {
	public delegate void UrlSendCallback (string url, bool success);
	[CCode (cname = "url_dispatch_send")]
	public void send (string url, UrlSendCallback? callback);
}
