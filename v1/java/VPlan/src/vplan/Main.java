package vplan;

public class Main {

    private static int farg;            //first commando argument
    private static PlanReader plan;     //Read the plan and make a txt
    private static Extractor extract;   //extract pieces of the plan

    public static void main(String[] args) {
        farg = Integer.parseInt(args[0]);
        plan = new PlanReader(farg);
        extract = new Extractor(plan.getvPlan());
        Persistence.writeCSVToFile(extract.convertElementsToString(), "vplan");
        Persistence.writeTxtToFile(extract.getData(), "meldungen");
        System.err.println(extract.convertElementsToString());
    }

}
