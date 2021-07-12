#include <gst/gst.h>

#define GST_USE_UNSTABLE_API

#include <gst/webrtc/webrtc.h>

static void on_offer_created (GstPromise * promise, GstElement * webrtc)
{
	g_print ("on_offer_created:\n");
	GstWebRTCSessionDescription *offer = NULL;
	const GstStructure *reply;
	gchar *desc;

	reply = gst_promise_get_reply (promise);
	gst_structure_get (reply, "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &offer, NULL);
	/* We can edit this offer before setting and sending */

	g_print(gst_sdp_message_as_text(offer->sdp));
	g_print(gst_webrtc_sdp_type_to_string(offer->type));

	g_signal_emit_by_name (webrtc, "set-local-description", offer, NULL);

	/* Implement this and send offer to peer using signalling */
//	send_sdp_offer (offer);
	gst_webrtc_session_description_free (offer);

}


static void on_negotiation_needed (GstElement * webrtc, gpointer user_data)
{
	g_print ("on_negotiation_needed:\n");

	GstPromise *promise;

	promise = gst_promise_new_with_change_func (on_offer_created,
	  user_data, NULL);
	g_signal_emit_by_name (webrtc, "create-offer", NULL,
	  promise);
}


static void send_ice_candidate_message (GstPromise * promise, GstElement * webrtc)
{
	g_print ("send_ice_candidate_message:\n");
}

static void on_incoming_stream (GstElement * webrtc, GstPad * pad, GstElement * pipe)
{
	g_print ("on_incoming_stream:\n");
}

static gboolean print_field (GQuark field, const GValue * value, gpointer pfx) {
  gchar *str = gst_value_serialize (value);

  g_print ("%s  %15s: %s\n", (gchar *) pfx, g_quark_to_string (field), str);
  g_free (str);
  return TRUE;
}


static void print_caps (const GstCaps * caps, const gchar * pfx) {
  guint i;

  g_return_if_fail (caps != NULL);

  if (gst_caps_is_any (caps)) {
    g_print ("%sANY\n", pfx);
    return;
  }
  if (gst_caps_is_empty (caps)) {
    g_print ("%sEMPTY\n", pfx);
    return;
  }

  for (i = 0; i < gst_caps_get_size (caps); i++) {
    GstStructure *structure = gst_caps_get_structure (caps, i);

    g_print ("%s%s\n", pfx, gst_structure_get_name (structure));
    gst_structure_foreach (structure, print_field, (gpointer) pfx);
  }
}


static void print_pad_capabilities (GstElement *element, gchar *pad_name) {
  GstPad *pad = NULL;
  GstCaps *caps = NULL;
  /* Retrieve pad */
  pad = gst_element_get_static_pad (element, pad_name);
  if (!pad) {
    g_printerr ("Could not retrieve pad '%s'\n", pad_name);
    return;
  }

  /* Retrieve negotiated caps (or acceptable caps if negotiation is not finished yet) */
  caps = gst_pad_get_current_caps (pad);
  if (!caps)
    caps = gst_pad_query_caps (pad, NULL);

  /* Print and free */
  g_print ("Caps for the %s pad:\n", pad_name);
  print_caps (caps, "      ");
  gst_caps_unref (caps);
  gst_object_unref (pad);
}

static gboolean bus_call (GstBus     *bus,
          GstMessage *msg, GMainLoop *loop)
{

//  guint64 running_time, stream_time;
//
//  GstState old_state, new_state, pending_state;
//  print_pad_capabilities (element, "sink");
//  gst_message_parse_qos(msg, NULL, &running_time, &stream_time, NULL, NULL);
//  gint64 current = -1;
//  g_print("%" G_GUINT64_FORMAT "\n", msg->timestamp);
//  g_print("%" G_GUINT64_FORMAT "\n", running_time);
//  g_print("%" G_GUINT64_FORMAT "\n", stream_time);
//
//  if (!gst_element_query_position (element, GST_FORMAT_TIME, &current)) {
//	g_printerr ("Could not query current position.\n");
//  }
//
//  g_print ("Position %" GST_TIME_FORMAT "/ %d \n",
//              GST_TIME_ARGS (current), GST_MESSAGE_TYPE (msg));
  gint64 current = -1;



  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);
      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_ELEMENT:
//    	if (!gst_element_query_position (element, GST_FORMAT_TIME, &current)) {
//    		g_printerr ("Could not query current position.\n");
//    	  }
//    	print_pad_capabilities (element, "sink");
//		g_print ("Position %" GST_TIME_FORMAT "/ %d \n",
//    	              GST_TIME_ARGS (current), GST_MESSAGE_TYPE (msg));
    	break;
    case GST_MESSAGE_LATENCY:
    	break;
    case GST_MESSAGE_STREAM_START:
    	break;
    case GST_MESSAGE_DURATION_CHANGED:
    	break;
    case GST_MESSAGE_HAVE_CONTEXT:
    	break;
//	case GST_MESSAGE_STATE_CHANGED:
//	  /* We are only interested in state-changed messages from the pipeline */
//		gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
//		g_print ("\nPipeline state changed from %s to %s:\n",
//		gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
//		/* Print the current capabilities of the sink element */
//	  break;
    default:
      break;
  }

  return TRUE;
}

int main(int   argc, char *argv[]) {
	GstElement *webrtc, *pipeline;
	gst_init(&argc, &argv);

	GstStateChangeReturn ret;

	pipeline = gst_pipeline_new ("webrtc");
	webrtc = gst_element_factory_make ("webrtcbin", "webrtcbin");

	if (!pipeline || !webrtc) {
	    g_printerr ("One element could not be created. Exiting.\n");
	    return -1;
	  }

	g_signal_connect (webrtc, "on-negotiation-needed",
	    G_CALLBACK (on_negotiation_needed), NULL);
	g_signal_connect (webrtc, "on-ice-candidate",
	    G_CALLBACK (send_ice_candidate_message), NULL);
	g_signal_connect (webrtc, "pad-added",
	    G_CALLBACK (on_incoming_stream), NULL);


	GstBus *bus;
	GMainLoop *loop;

	loop = g_main_loop_new (NULL, FALSE);

	gst_bin_add_many (GST_BIN (pipeline), webrtc, NULL);

	ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr ("Unable to set the pipeline to the playing state (check the bus for error messages).\n");
	}


	bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
	gst_bus_add_signal_watch (bus);
	g_signal_connect (bus, "message", G_CALLBACK (bus_call), loop);
	g_main_loop_run (loop);



	g_print ("aaaa:\n");
}
