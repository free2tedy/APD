import java.util.Hashtable;
import java.util.LinkedList;

/**
 *
 * @author Tudor Dimcica
 */
public class WorkPool {

    LinkedList<MapWork> mapTasks;
    LinkedList<ReduceWork> reduceTasks;

    public WorkPool() {
        mapTasks = new LinkedList<>();
        reduceTasks = new LinkedList<>();
    }

    synchronized void putMapWork(MapWork mw) {
        mapTasks.add(mw);
        this.notify();
    }

    MapWork getMapWork() {
        if (mapTasks.size() != 0) {
            return mapTasks.remove();
        } else {
            return null;
        }
    }

    synchronized void putReduceWork(ReduceWork rw) {
        reduceTasks.add(rw);
        this.notify();
    }

    ReduceWork getReduceWork() {
        if (reduceTasks.size() != 0) {
            return reduceTasks.remove();
        } else {
            return null;
        }
    }
}

class MapWork {

    String fileName;
    String workLoad;

    public MapWork(String fileName, String workLoad) {
        this.fileName = fileName;
        this.workLoad = workLoad;
    }
}

class ReduceWork {

    String fileName;
    Hashtable<String, Integer> workLoad;

    public ReduceWork(String fileName, Hashtable workLoad) {
        this.workLoad = workLoad;
        this.fileName = fileName;
    }
}
