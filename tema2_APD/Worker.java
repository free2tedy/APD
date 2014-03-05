import java.text.DecimalFormat;
import java.util.Hashtable;
import java.util.Set;
import java.util.StringTokenizer;

/**
 *
 * @author Tudor Dimcica
 */
public class Worker extends Thread {

    WorkPool wp;
    /*
     * type = 0 --> MAP
     * type = 1 --> REDUCE
     */
    int type;
    static Hashtable<String, Hashtable<String, Integer>> mapReduce = new Hashtable<>();
    static Hashtable<String, Integer> wordNumber = new Hashtable<>();
    static Hashtable<String, Double> similarityFactor = new Hashtable<>();
    static String baseText;

    public Worker(WorkPool wp, int type, String baseText) {
        this.wp = wp;
        this.type = type;
		  this.baseText = baseText;
    }

    public Hashtable<String, Integer> createHashTable(MapWork mp) {
        StringTokenizer st = new StringTokenizer(mp.workLoad, " _().,;:-\"\n\r\t\f");
        Hashtable<String, Integer> freqTable = new Hashtable<>();

        synchronized (wordNumber) {
            if(wordNumber.get(mp.fileName) == null) {
                wordNumber.put(mp.fileName, 0);
            }
        }
		
        int count = 0;
        while (st.hasMoreTokens()) {
        		count++;
            String buf = st.nextToken().toLowerCase();
            if (freqTable.get(buf) != null) {
                freqTable.put(buf, freqTable.get(buf) + 1);
            } else {
                freqTable.put(buf, 1);
            }
        }
		
        synchronized(wordNumber) {
			wordNumber.put(mp.fileName, wordNumber.get(mp.fileName) + count);
        }

        return freqTable;
    }

    @Override
    public void run() {
        while (true) {
            MapWork mw;
            if (type == 0) {
                /* 
                 * Verific daca mai exista taskuri de map.
                 * Daca da, scot un task pentru a il prelucra.
                 */
                synchronized (wordNumber) {
                    if (wp.mapTasks.size() == 0) {
                        break;
                    } else {
                        mw = wp.getMapWork();
                        if (mw == null) {
                            break;
                        }
                    }
                }

                /* 
                 * Verific daca exista hashtable pentru
                 * fisierul taskului prelucrat.
                 */
                synchronized (wordNumber) {
                    if (mapReduce.get(mw.fileName) == null) {
                        mapReduce.put(mw.fileName, new Hashtable<String, Integer>());
                    }
                }

                /* 
                 * Prelucrez si obtin hashtableul asociat taskului.
                 */
                Hashtable<String, Integer> hashTask = createHashTable(mw);

                /*
                 * Incep sa iterez prin cheile obtinute
                 * in hashtableul taskului si updatez 
                 * hashtableul fisierului.
                 */
                Set<String> keys = hashTask.keySet();
                for (String s : keys) {
                    synchronized (wordNumber) {
                        if (mapReduce.get(mw.fileName).get(s) == null) {
                            mapReduce.get(mw.fileName).put(s, hashTask.get(s));
                        } else {
                            mapReduce.get(mw.fileName).put(s, mapReduce.get(mw.fileName).get(s) + hashTask.get(s));
                        }
                    }
                }
            }

            if (type == 1) {
                Double sum = 0.0;
                ReduceWork rw;

            	 /* 
                 * Verific daca mai exista taskuri de reduce.
                 * Daca da, scot un task pentru a il prelucra.
                 */
                synchronized (wordNumber) {
                    if (wp.reduceTasks.size() == 0) {
                        break;
                    } else {
                        rw = wp.getReduceWork();
                        if (rw == null) {
                            break;
                        }
                    }
                }

					 /*
					  * Iterez prin toate cheile si calculez
					  * factorul de similitudine.
					  */
                Set<String> keys = rw.workLoad.keySet();
                for (String s : keys) {
                    if (mapReduce.get(baseText).get(s) != null) {
                        float x = ((float) mapReduce.get(baseText).get(s)) / ((float) wordNumber.get(baseText));
                        float y = ((float) rw.workLoad.get(s)) / ((float) wordNumber.get(rw.fileName));
                        sum += x * y;
                    }
                }
                similarityFactor.put(rw.fileName, sum * 100);
            }

        }
    }
}
