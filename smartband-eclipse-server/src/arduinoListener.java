

import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import org.json.simple.JSONObject;

/**
 * Servlet implementation class arduinoListener
 */
@WebServlet("/arduinoListener")
public class arduinoListener extends HttpServlet {
	private static final long serialVersionUID = 1L;
       
    /**
     * @see HttpServlet#HttpServlet()
     */
    public arduinoListener() {
        super();
        // TODO Auto-generated constructor stub
    }

	/**
	 * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		JSONObject obj = new JSONObject();
		HttpSession session = request.getSession(true);
        	
        	String UV_string = request.getParameter("UV");
        	if (UV_string == null) 
        		UV_string = "0";
       		
        	String step_string = request.getParameter("step");
        	if (step_string == null)
        		step_string = "0";
        	
        	String temperature_string = request.getParameter("temperature");
        	if (temperature_string == null) 
        		temperature_string = "0";
        	
        	String carbon_string = request.getParameter("carbon");
        	if (carbon_string == null)
        		carbon_string = "0";
        	
        	String heartbeat_string = request.getParameter("heartbeat");
        	if (heartbeat_string == null)
        		heartbeat_string = "0";
        	
        	obj.put("UV", UV_string);
        	obj.put("temperature", temperature_string);
        	obj.put("carbon", carbon_string);
        	obj.put("step", step_string);
        	obj.put("heartbeat", heartbeat_string);
        	
        	try (FileWriter file = new FileWriter("/Users/yifan/eclipse-workspace/smartband/file1.txt")) {
    			file.write(obj.toJSONString());
    			System.out.println("Successfully Copied JSON Object to File...");
    			System.out.println("\nJSON Object: " + obj);
    		}
        	
	}

	/**
	 * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		// TODO Auto-generated method stub
		doGet(request, response);
	}

}
