public class doolhof
{


    enum sas
{
    UNKOWN ,
            LEFT ,
            FORWARD ,
            RIGHT ,
            TURNAROUND ,
            CHECKJUNCTION ,
            BACKWARD ,
            FINISHED
};

    void optimizeRoute() {
        for (int i = 1; i < path.length && path[i] != sas.UNKOWN; i++) {
            if (path[i] == sas.TURNAROUND) {
                sas before = path[i - 1];
                sas after = path[i + 1];
                sas result = null;
                switch (before) {
                    case LEFT:
                        if (after == sas.LEFT) {
                            result = sas.FORWARD;
                        }
                        else if (after == sas.FORWARD) {
                            result = sas.RIGHT;
                        }
                        else if (after == sas.RIGHT) {
                            result = sas.TURNAROUND;
                        }
                        break;
                    case FORWARD:
                        if (after == sas.LEFT) {
                            result = sas.RIGHT;
                        }
                        else if (after == sas.FORWARD) {
                            result = sas.TURNAROUND;
                        }
                        break;
                    case RIGHT:
                        result = sas.TURNAROUND;
                        break;
                    default:
                        break;
                }
                path[i - 1] = result;
                for (int j = i; j < path.length-2; j++) {
                    path[j] = path[j + 2];
                    path[j+2] = sas.UNKOWN;
                }
                i = 0;
            }
        }
    }




    sas[] path = { sas.LEFT, sas.LEFT, sas.RIGHT,
            sas.LEFT, sas.TURNAROUND, sas.LEFT,
            sas.TURNAROUND, sas.LEFT, sas.TURNAROUND, sas.LEFT,sas.LEFT,sas.LEFT};


    void printPath(){
        for (sas s: path
             ) {
            System.out.println(s.toString());
        }
    }

    public static void main(String[] args) {
        doolhof d= new doolhof();
        d.printPath();
        System.out.println("optimazed:");
        d.optimizeRoute();
        d.printPath();

    }

    /**
     *
     */
}
