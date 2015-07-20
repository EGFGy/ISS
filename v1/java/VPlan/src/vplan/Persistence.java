package vplan;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Persistence {

    private static FileWriter writer;
    private static File file;
    private static String work;

    public Persistence(String work) {
        Persistence.work = work;
    }

    public Persistence() {
        work = "";
    }

    public static void writeTxtToFile(String txt, String title) {
        try {
            file = new File(title);
            writer = new FileWriter(file, false);
            writer.write(txt);
            writer.flush();
            writer.close();
        } catch (IOException ex) {
            Logger.getLogger(PlanReader.class.getName()).log(Level.SEVERE, null, ex);
        }

    }
    
    public static void writeCSVToFile(String txt, String title) {
        try {
            file = new File(title + ".csv");
            writer = new FileWriter(file, false);
            writer.write(txt);
            writer.flush();
            writer.close();
        } catch (IOException ex) {
            Logger.getLogger(PlanReader.class.getName()).log(Level.SEVERE, null, ex);
        }

    }

    public static String getWork() {
        return work;
    }

    public static void setWork(String work) {
        Persistence.work = work;
    }
}
