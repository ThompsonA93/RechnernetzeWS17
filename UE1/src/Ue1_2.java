import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.Scanner;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

public class Ue1_2 {
    String inFile;
    String outFile;
    BufferedWriter bw;
    BufferedReader br;
    File tmp;

    public void init(){
        System.out.println("Executing UE1_2.");
        Scanner s = new Scanner(System.in);
        System.out.println("Enter Filename to convert.");
        inFile = s.nextLine();
        System.out.println("Enter outputname.");
        outFile = s.nextLine();

        initFiles();
        convertToISO();

        try{
            bw.close();
            br.close();
        }catch(Exception e){
            e.printStackTrace();
        }

        System.out.println("End of UE1_2");
    }

    private void initFiles() {
        try {
            br = new BufferedReader(new FileReader(inFile));
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        try {
            // temporary file where converted text will be stored
            tmp = File.createTempFile("convert", ".tmp");
            tmp.deleteOnExit();

            // OutputStreamWriter can handle different charsets, so use that
            bw = new BufferedWriter((new OutputStreamWriter(new FileOutputStream(tmp), StandardCharsets.ISO_8859_1)));

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void convertToISO() {
        String read;

        try {
            while ((read = br.readLine()) != null) {
                bw.write(read);
                bw.write("\r\n"); // also write CRLF. There is nothing to replace as this reader doesn't add any line separator
            }
            bw.flush();

        } catch (IOException e) {
            e.printStackTrace();
        }

        zipCompression();
    }

    private void zipCompression() {
        byte[] buffer = new byte[1000];
        int bytesRead;

        try {
            ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(outFile));
            FileInputStream fis = new FileInputStream(tmp);

            zos.putNextEntry(new ZipEntry("converted.txt"));

            while ((bytesRead = fis.read(buffer)) != -1) {
                zos.write(buffer, 0, bytesRead);
            }

            zos.closeEntry();
            zos.finish();
            zos.close();
            fis.close();

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
