package main;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import edu.dartmouth.cs.zebradb.common.ObjectIO;
import edu.dartmouth.cs.zebradb.match.*;
import newsift.*;

public class WildIDBatchCompare {
	WildIDBatchCompare(String basePath) {
		this.basePath = basePath;
	}
	
	private double scorePair(String name1, String name2)
	{
		PhotoSummary.ValidSummary vs1 = photos.get(name1);
		PhotoSummary.ValidSummary vs2 = photos.get(name2);
		
		if (vs1 == null || vs2 == null)
		{
			return -10;
		}
		
		double score = new MyMatch().matchFeatures(vs1.siftEncoding, vs2.siftEncoding, null, null).getDoubleTriangleScore();
		return score;
	}

	private Map<String, PhotoSummary.ValidSummary> photos;
	private String basePath;
	
	public static void main(String[] args) throws IOException, ClassNotFoundException {
		String basePath = "/home/mmatthe/programming/AmphIdent_misc/DBsForPaper/salamandra_joined/WildID/match_info";
		new WildIDBatchCompare(basePath).run();
	}
	
	private List<String> loadIndividuals() throws IOException
	{
		List<String> result = new ArrayList<String>();
		String indfileName = basePath + "/../individuals.txt";
		
		FileInputStream fstream = new FileInputStream(indfileName);
		BufferedReader br = new BufferedReader(new InputStreamReader(fstream));
		
		String strLine;
		
		while((strLine = br.readLine()) != null) {
			String[] parts = strLine.split(" ");
			for (String p : parts) {
				result.add(p);
			}
		}
		br.close();
		return result;
	}
	
	private class MyFileWriter {
		MyFileWriter(String outName) throws IOException {
			writer = new BufferedWriter(new FileWriter(new File(outName)));
		}
		
		BufferedWriter writer;
		
		public synchronized void write(String line) throws IOException
		{
			writer.write(line + "\n");
			//System.out.println(line);
		}
		
		public void close() throws IOException {
			writer.close();
		}
	}
	
	public void scoreIndividual(String name, MyFileWriter writer) throws IOException{
		for(String ind2 : photos.keySet()) {
			double score = scorePair(name, ind2);
			//System.out.println(ind1 + " " + ind2 + " " + score);
			writer.write(name + " " + ind2 + " " + score);
		}
	}
	
	class WorkerThread implements Runnable {
		public WorkerThread(String name, WildIDBatchCompare main, String basePath) {
			this.name = name;
			this.main = main;
			this.basePath = basePath;
		}
		String name;
		WildIDBatchCompare main;
		String basePath;
		
		public void run()
		{
			try {
				MyFileWriter writer = new MyFileWriter(basePath + "/../comparisonTable" + name + ".txt");
				main.scoreIndividual(name, writer);
				writer.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
	
	public void run() throws IOException, ClassNotFoundException {
		List<Path> files = Files.walk(Paths.get(basePath))
				.filter(w -> w.toString().contains("siftsumm_"))
				.filter(w -> w.toFile().isFile())
				.collect(Collectors.toList());
		photos = new HashMap<String, PhotoSummary.ValidSummary>();
		for(Path file : files)
		{
			PhotoSummary.SerialSummary s = (PhotoSummary.SerialSummary) ObjectIO.readJavaSerial(file.toFile());
			PhotoSummary.ValidSummary vs = (PhotoSummary.ValidSummary)s;
			photos.put(s.photoRelativePath.toString(), vs);
		}
	
		List<String> individuals = loadIndividuals();
		System.out.println("Found " + photos.size() + " photo summaries...");
		System.out.println("Found " + individuals.size() + " individuals...");
		
		int index = 0;
		//MyFileWriter writer = new MyFileWriter(basePath + "/../comparisonTable.txt");
		ExecutorService executor = Executors.newFixedThreadPool(8);
		for(String ind1: individuals) {
			System.out.println(index + " " + ind1);
			//scoreIndividual(ind1, writer);
			Runnable r = new WorkerThread(ind1, this, basePath);
			executor.execute(r);
			index = index+1;
		}
		executor.shutdown();
		while (!executor.isTerminated())
			;
		System.out.println("Done comparing");
		

		
	}

}
