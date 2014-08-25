var http = require("http");
var url = require("url");

function start(route) {
  function onRequest(request, response) {
    var path = url.parse(request.url).path;
    console.log("Request for " + path);
    route(path);

    response.writeHead(200, {"Content-Type": "text/html"});
    response.write("<html><head><title>ToSa Mesh</title></head><body>");
    response.write("<form method=\"get\" action=\"/cmd.digitalwrite\">");
    response.write("Node Address:&nbsp;<input name=\"nodeAddress\" type=\"text\" value=\"3\" /><br>");
    response.write("Color:&nbsp;<select name=\"pinNumber\">");
    response.write("<option value=\"7\">red</option>");
    response.write("<option value=\"6\">green</option>");
    response.write("</select><br>");
    response.write("State:&nbsp;<select name=\"pinState\">");
    response.write("<option value=\"1\">on</option>");
    response.write("<option value=\"0\">off</option>");
    response.write("</select><br>");
    response.write("<input type=\"submit\" value=\"Digital Write\" />");
    response.write("</form>");
    response.write("<br>");
    response.write("<form method=\"get\" action=\"/cmd.setevent\">");
    response.write("Node Address:&nbsp;<input name=\"nodeAddress\" type=\"text\" value=\"3\" /><br>");
    response.write("Timer Slot:&nbsp;<input name=\"timerSlot\" type=\"text\" value=\"0\" /><br>");
    response.write("Weekdays:&nbsp;<input name=\"weekday\" type=\"checkbox\" value=\"1\" checked=\"checked\"/>Mo");
    response.write("&nbsp;<input name=\"weekday\" type=\"checkbox\" value=\"2\" checked=\"checked\"/>Di");
    response.write("&nbsp;<input name=\"weekday\" type=\"checkbox\" value=\"3\" checked=\"checked\"/>Mi");
    response.write("&nbsp;<input name=\"weekday\" type=\"checkbox\" value=\"4\" checked=\"checked\"/>Do");
    response.write("&nbsp;<input name=\"weekday\" type=\"checkbox\" value=\"5\" checked=\"checked\"/>Fr");
    response.write("&nbsp;<input name=\"weekday\" type=\"checkbox\" value=\"6\" />Sa");
    response.write("&nbsp;<input name=\"weekday\" type=\"checkbox\" value=\"0\" />So<br>");
    response.write("Hour:&nbsp;<input name=\"hour\" type=\"text\" value=\"13\" />&nbsp;Minute:&nbsp;<input name=\"minute\" type=\"text\" value=\"20\" /><br>");
//    response.write("Color:&nbsp;<select name=\"pinNumber\">");
//    response.write("<option value=\"7\">red</option>");
//    response.write("<option value=\"6\">green</option>");
//    response.write("</select><br>");
//    response.write("State:&nbsp;<select name=\"pinState\">");
//    response.write("<option value=\"1\">on</option>");
//    response.write("<option value=\"0\">off</option>");
//    response.write("</select><br>");
    response.write("State:&nbsp;<input name=\"stateId\" type=\"text\" value=\"0\" /><br>");
    response.write("<input type=\"submit\" value=\"Set Event\" />");
    response.write("</form>");
    response.write("<br>");
    response.write("<form method=\"get\" action=\"/cmd.getevent\">");
    response.write("Node Address:&nbsp;<input name=\"nodeAddress\" type=\"text\" value=\"3\" /><br>");
    response.write("Timer Slot:&nbsp;<input name=\"timerSlot\" type=\"text\" value=\"0\" /><br>");
    response.write("<input type=\"submit\" value=\"Get Event\" />");
    response.write("</form>");
    response.write("<br>");
    response.write("<form method=\"get\" action=\"/cmd.setstate\">");
    response.write("Node Address:&nbsp;<input name=\"nodeAddress\" type=\"text\" value=\"3\" /><br>");
    response.write("State:&nbsp;<input name=\"stateId\" type=\"text\" value=\"0\" /><br>");
    response.write("Red LED state:&nbsp;<select name=\"stateLed\">");
    response.write("<option value=\"1\">on</option>");
    response.write("<option value=\"0\">off</option>");
    response.write("</select><br>");
    response.write("<input type=\"submit\" value=\"Set State\" />");
    response.write("</form>");
    response.write("<br>");
    response.write("<form method=\"get\" action=\"/cmd.getstate\">");
    response.write("Node Address:&nbsp;<input name=\"nodeAddress\" type=\"text\" value=\"3\" /><br>");
    response.write("Timer Slot:&nbsp;<input name=\"stateId\" type=\"text\" value=\"0\" /><br>");
    response.write("<input type=\"submit\" value=\"Get State\" />");
    response.write("</form>");
    response.write("<br>");
    response.write("<form method=\"get\" action=\"/cmd.sendtime\">");
    response.write("<input type=\"submit\" value=\"Send Time\" />");
    response.write("</form>");
    response.write("</body></html>");
	
//    response.writeHead(200, {"Content-Type": "text/plain"});
//    response.write("ToSa Mesh Homepage");

    response.end();
  }
  http.createServer(onRequest).listen(8888);
  console.log("Server has started.");
}

exports.start = start;
