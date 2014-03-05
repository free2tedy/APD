import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Map;
import java.util.Scanner;
import java.util.Set;

/**
 *
 * @author Tudor Dimcica
 */
public class MapReduce {

    String baseText;
    String[] compText;
    int D, ND, nThreads;
    float X;
    WorkPool wp;
    static int MAP = 0, REDUCE = 1;

    public MapReduce(String inputPath, int nThreads) throws FileNotFoundException {
        readInput(inputPath);
        this.nThreads = nThreads;
        wp = new WorkPool();
    }

    public void readInput(String inputPath) throws FileNotFoundException {
        Scanner in = new Scanner(new File(inputPath));
        baseText = new String(in.next());
        D = in.nextInt();
        X = in.nextFloat();
        ND = in.nextInt();
        compText = new String[ND];
        for (int i = 0; i < ND; i++) {
            compText[i] = new String(in.next());
        }
        in.close();
    }

    public void readDoc(String filePath) throws FileNotFoundException, IOException {
        FileInputStream fip = new FileInputStream(new File(filePath));
        byte[] buffer;
        String sBuffer;
        int count;

        while (fip.available() >= D) {
            buffer = new byte[2 * D];
            fip.read(buffer, 0, D);
            count = -1;
            while (buffer[D + count] != 32 && buffer[D + count] != 10) {
                count++;
                int readByte = fip.read();
                if (readByte != -1) {
                    buffer[D + count] = (byte) readByte;
                }
            }
            sBuffer = new String(buffer, 0, D + count);
            wp.putMapWork(new MapWork(filePath, sBuffer));
        }

        buffer = new byte[fip.available()];
        fip.read(buffer, 0, fip.available());

        sBuffer = new String(buffer);
        wp.putMapWork(new MapWork(filePath, sBuffer));

        fip.close();
    }

    public void readAll() throws FileNotFoundException, IOException {
        readDoc(baseText);

        for (int i = 0; i < ND; i++) {
            if (compText[i].compareTo(baseText) != 0) {
                readDoc(compText[i]);
            }
        }

    }

    public static void main(String[] args) throws FileNotFoundException, IOException, InterruptedException {
        MapReduce mp = new MapReduce(args[1], Integer.parseInt(args[0]));
        mp.readAll();

        Worker wk[] = new Worker[mp.nThreads];
        for (int i = 0; i < wk.length; i++) {
            wk[i] = new Worker(mp.wp, MAP, mp.baseText);
        }
        for (int i = 0; i < wk.length; i++) {
            wk[i].start();
        }
        for (int i = 0; i < wk.length; i++) {
            wk[i].join();
        }
        
        
        for (int i = 0; i < mp.ND; i++) {
            mp.wp.putReduceWork(new ReduceWork(mp.compText[i], Worker.mapReduce.get(mp.compText[i])));
        }
                
        for (int i = 0; i < wk.length; i++) {
            wk[i] = new Worker(mp.wp, REDUCE, mp.baseText);
        }
        for (int i = 0; i < wk.length; i++) {
            wk[i].start();
        }
        for (int i = 0; i < wk.length; i++) {
            wk[i].join();
        }

        

        ArrayList<Pair> simValues = new ArrayList();
        for (String s : mp.compText) {
            simValues.add(new Pair(s, Worker.similarityFactor.get(s)));
        }
        Collections.sort(simValues, new Comparator<Pair>() {
            @Override
            public int compare(Pair t, Pair t1) {
                if (t.x == t1.x) {
                    return 0;
                } else if (t.x < t1.x) {
                    return 1;
                } else {
                    return -1;
                }
            }
        });


        PrintWriter pw = new PrintWriter(new File(args[2]));
        pw.format("Rezultate pentru: (%s)\n\n", mp.baseText);
        for(int i = 0; i < simValues.size(); i++) {
            if(simValues.get(i).fileName.compareTo(mp.baseText) != 0 
                    && simValues.get(i).x >= mp.X) {
                pw.format("%s (%.3f%%)\n", simValues.get(i).fileName, simValues.get(i).x);
            }           
        }

        pw.close();
    }
}

class Pair {

    String fileName;
    Double x;

    public Pair(String fileName, Double x) {
        this.fileName = fileName;
        this.x = x;
    }

    public String toString() {
        return "(" + fileName + "," + String.format("%.3f", x) + ")";
    }
}
