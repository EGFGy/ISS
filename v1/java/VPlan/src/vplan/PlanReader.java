package vplan;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.MalformedURLException;
import java.net.Proxy;
import java.net.URL;
import java.net.URLConnection;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Liest die Website, schreibt den Seitencode in deine Datei und in eine
 * weiternutzbare Stringvariabel.
 *
 * @author 11MonsR
 */
public class PlanReader {

    private static String vPlan = null;            //Saving the String temporaly
    private static URL urlOb;                      //URL-Object for working and representation
    private static URLConnection connect;          //establish an URL-Connection
    private static Proxy proxy = new Proxy(Proxy.Type.HTTP, new InetSocketAddress("in-gate.egf.local", 3128));

    PlanReader(int weekday) {
        String day = giveURLOfDay(weekday);                   //gives appropiate URL to the weekday
        extractTextfromWeb(day);                              //extracts Text from a website
    }

    /**
     * Schreibt den Websiteninhalt in die Variable vPlan
     *
     * @param url String der Website, die konvertiert werden soll
     */
    private static void extractTextfromWeb(String url) {
        try {
            urlOb = new URL(url);
            HttpURLConnection uc = (HttpURLConnection)urlOb.openConnection(proxy);
            uc.connect();
            InputStreamReader lineReader = new InputStreamReader(uc.getInputStream());
            BufferedReader buffer = new BufferedReader(lineReader);
            String inputLine = null;
            while ((inputLine = buffer.readLine()) != null) {
                vPlan += "\n" + inputLine;                  //buffer.readLine() is bullshit!
            }
            buffer.close();
        } catch (MalformedURLException ex) {
            Logger.getLogger(PlanReader.class.getName()).log(Level.SEVERE, null, ex);
        } catch (IOException ex) {
            Logger.getLogger(PlanReader.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    /**
     * @param day Eingabe des Tages: Montag(1), Dienstag(2), ...
     * @return URL des gewünschten Tages
     */
    private static String giveURLOfDay(int day) {
        String url = null;
        switch (day) {
            case 1:
                url = "http://vertretungsplan.egf-online.de/Schuelerplanjoomla/schuelerplan_mo.htm";
                break;
            case 2:
                url = "http://vertretungsplan.egf-online.de/Schuelerplanjoomla/schuelerplan_di.htm";
                break;
            case 3:
                url = "http://vertretungsplan.egf-online.de/Schuelerplanjoomla/schuelerplan_mi.htm";
                break;
            case 4:
                url = "http://vertretungsplan.egf-online.de/Schuelerplanjoomla/schuelerplan_do.htm";
                break;
            case 5:
                url = "http://vertretungsplan.egf-online.de/Schuelerplanjoomla/schuelerplan_fr.htm";
                break;
            default:
        }
        return url;
    }

    /**
     * @return vPlan einen String mit dem Quellcode
     */
    public static String getvPlan() {
        return vPlan;
    }
}
