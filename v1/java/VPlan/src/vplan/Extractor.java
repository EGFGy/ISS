package vplan;

import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;
//WILL NEED SOME REFACTORING AND REWRITING IN THE END!!!
public class Extractor {

    private static final String name = "Ehrenbürg-Gymnasium Forchheim";
    private static String stand, messages, tag, convStr;
    private static final List<Element> elements = new ArrayList<>();
    private static final ArrayList<String> lines = new ArrayList<>();

    public Extractor(String str) {
        Extractor.convStr = str;
        extractTitel(convStr);
        convStr = removeWeirdness(convStr, "ä");
        extractStand(convStr);
        getMessages(convStr);
        convStr = getPartOfString(convStr, "<table class=\"TabelleVertretungen\" cellpadding=\"2px\">", "</Body>");
        createElements(convStr);
    }

    private void createLineList(String str) {
        lines.clear();
        StringTokenizer st = new StringTokenizer(str, "\n");
        while (st.hasMoreTokens()) {
            lines.add(st.nextToken());
        }
    }

    private void createElements(String str) {
        createLineList(str);
        String vertretung = "";
        String std;
        String klasse = "";
        String raum = "";
        String fach = "";
        String statt = "";
        String lehrer = "";
        String sonstiges = "";
        for (int z = 0; z < lines.size() / 9; z++) {
            int i = 9 * z;
            String zeile = lines.get(i);
            zeile = lines.get(i + 1);
            klasse = getPartOfString(zeile, "Vertretungen\">", "<");
            zeile = lines.get(i + 2);
            std = getPartOfString(zeile, "Vertretungen\">", "<");
            zeile = lines.get(i + 3);
            vertretung = getPartOfString(zeile, "Vertretungen\">", "<");
            zeile = lines.get(i + 4);
            fach = getPartOfString(zeile, "Vertretungen\">", "<");
            zeile = lines.get(i + 5);
            raum = getPartOfString(zeile, "Vertretungen\">", "<");
            zeile = lines.get(i + 6);
            statt = getPartOfString(zeile, "Vertretungen\">", "<");
            zeile = lines.get(i + 7);
            lehrer = getPartOfString(zeile, "Vertretungen\">", "<");
            zeile = lines.get(i + 8);
            sonstiges = getPartOfString(zeile, "Vertretungen\">", "<");
            Element item = new Element(klasse, std, vertretung, fach, raum, statt, lehrer, sonstiges);
            elements.add(item);
        }
        if (elements.size() > 0) {
            elements.remove(0);
        }
    }

    private void extractStand(String str) {
        stand = getPartOfString(str, "Stand:", "<");
        System.out.println(stand);
    }

    private void getMessages(String str) {
        String work = getPartOfString(str, "<td>", "</td");
        work = delBreak(work);
        messages = work;

    }

    private String delBreak(String s) {
        s = s.replaceAll("<BR /> <BR /> ", "\n");
        s = s.replaceAll("<BR />", "\n");
        return s;
    }

    private void extractTitel(String str) {
        tag = getPartOfString(str, "<Title>", "</Title");
        tag = removeWeirdness(tag, "ü");
        String s = "Gymnasium Ehrenbürg Forchheim Vertretungsplan für";
        int len = s.length();
        if (tag.startsWith(s)) {
            tag = tag.substring(len);
        } else {
            tag = "VP liegt nicht vor.";
        }
    }

    public static String getPartOfString(String s, String s1, String s2) {
        String erg = "";
        int m = s.indexOf(s1);
        if (m > -1) {
            String from = s.substring(m + s1.length());
            int n = from.indexOf(s2);
            if (n > -1) {
                erg = s.substring(m + s1.length(), m + s1.length() + n).trim();
            }
        }
        return erg;
    }

    private String removeWeirdness(String s, String weird) {
        s = s.replaceAll("�", weird);
        return s;
    }

    public String convertElementsToString() {
        String work = "";
//        work = "Tag:," + tag.replace(",", "") + "\n";
//        work = work + "Stand:," + stand + "\n\n";
//        work = work + "Messages:,\n" + messages.replace("ä", "�") + "\n\n";
        work = work + "klasse,std,vertretung,fach,raum,statt,lehrer,sonstiges" + "\n";
        for (Element e : elements) {
            work += e.convertToCSV();
            work += "\n";
        }
        return work;
    }
    
    public String getData(){
        String work = "";
        work = "Tag: " + tag.replace(",", "") + "\n";
        work = work + "Stand: " + stand + "\n\n";
        work = work + "Nachrichten:\n" + messages.replace("ä", "") + "\n\n";
        return work;
    }

    public static List<Element> getElements() {
        return elements;
    }

    public static String getStand() {
        return stand;
    }

    public static void setStand(String stand) {
        Extractor.stand = stand;
    }

    public static String getMessages() {
        return messages;
    }

    public static void setMessages(String messages) {
        Extractor.messages = messages;
    }

    public static String getTag() {
        return tag;
    }

    public static void setTag(String tag) {
        Extractor.tag = tag;
    }

    public static String getConvStr() {
        return convStr;
    }

    public static void setConvStr(String convStr) {
        Extractor.convStr = convStr;
    }

    private class Element {

        private String klasse = "null";
        private String std = "null";
        private String vertretung = "null";
        private String fach = "null";
        private String raum = "null";
        private String statt = "null";
        private String lehrer = "null";
        private String sonstiges = "null";

        public String getKlasse() {
            return klasse;
        }

        public String getStd() {
            return std;
        }

        public String getVertretung() {
            return vertretung;
        }

        public String getFach() {
            return fach;
        }

        public String getRaum() {
            return raum;
        }

        public String getStatt() {
            return statt;
        }

        public String getLehrer() {
            return lehrer;
        }

        public String getSonstiges() {
            return sonstiges;
        }

        public String removeSpace(String s) {
            if (s.equals("")) {
                return "null";
            } else {
                return s;
            }
        }

        public Element(String klasse, String std, String vertretung, String fach, String raum, String statt, String lehrer, String sonstiges) {
            this.klasse = klasse;
            this.std = std;
            this.vertretung = vertretung;
            this.fach = fach;
            this.raum = raum;
            this.statt = statt;
            this.lehrer = lehrer;
            this.sonstiges = sonstiges;
        }

        @Override
        public String toString() {
            return "Element{" + "klasse=" + klasse + ", std=" + std + ", vertretung=" + vertretung + ", fach=" + fach + ", raum=" + raum + ", statt=" + statt + ", lehrer=" + lehrer + ", sonstiges=" + sonstiges + '}';
        }

        public String convertToCSV() {
            klasse = removeSpace(klasse);
            std = removeSpace(std);
            vertretung = removeSpace(vertretung);
            fach = removeSpace(fach);
            raum = removeSpace(raum);
            statt = removeSpace(statt);
            lehrer = removeSpace(lehrer);
            sonstiges = removeSpace(sonstiges);
            return klasse + "," + std + "," + vertretung + "," + fach.replace("ä", "Ü") + "," + raum + "," + statt + "," + lehrer + "," + sonstiges;
        }
    }
}
