import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class Tema2 {

    private static int NUM_THREADS;
    private static ExecutorService PRIMARY_EXECUTOR_SERVICE;
    private static ExecutorService SECONDARY_EXECUTOR_SERVICE;

    public static void main(String[] args) throws IOException, InterruptedException {
        NUM_THREADS = Integer.parseInt(args[1]);
        PRIMARY_EXECUTOR_SERVICE = Executors.newFixedThreadPool(NUM_THREADS);
        SECONDARY_EXECUTOR_SERVICE = Executors.newFixedThreadPool(NUM_THREADS);
        String fileName = args[0] + "/orders.txt";
        String productsFilePath = args[0] + "/order_products.txt";
        String ordersOut = "orders_out.txt";
        String orderProductsOut = "order_products_out.txt";
        File file = new File(orderProductsOut);
        if (file.exists()) {
            file.delete();
        }
        file = new File(ordersOut);
        if (file.exists()) {
            file.delete();
        }
        BufferedReader reader = new BufferedReader(new FileReader(fileName));
        String line;
        while ((line = reader.readLine()) != null) {
            PRIMARY_EXECUTOR_SERVICE.submit(new PrimaryWorker(line, productsFilePath, ordersOut, orderProductsOut));
        }
        reader.close();

        PRIMARY_EXECUTOR_SERVICE.shutdown();
        PRIMARY_EXECUTOR_SERVICE.awaitTermination(1, TimeUnit.HOURS);

        SECONDARY_EXECUTOR_SERVICE.shutdown();
        SECONDARY_EXECUTOR_SERVICE.awaitTermination(1, TimeUnit.HOURS);
    }

    private static class PrimaryWorker implements Runnable {
        private final String line;
        private final String productsFilePath;
        private final String ordersOut;
        private final String orderProductsOut;
        private int remainingWorkers;

        private PrimaryWorker(String line, String productsFilePath, String ordersOut, String orderProductsOut) {
            this.line = line;
            this.productsFilePath = productsFilePath;
            this.ordersOut = ordersOut;
            this.orderProductsOut = orderProductsOut;
        }

        @Override
        public void run() {
            String[] parts = line.split(",");
            String orderId = parts[0];
            int noOfProducts = Integer.parseInt(parts[1]);
            remainingWorkers = noOfProducts;

            for (int i = 0; i < noOfProducts; i++) {
                SECONDARY_EXECUTOR_SERVICE
                        .submit(new SecondaryWorker(orderId, noOfProducts, productsFilePath, ordersOut,
                                orderProductsOut,
                                new AtomicInteger(i), this));
            }
            synchronized (this) {
                while (remainingWorkers > 0) {
                    try {
                        wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
            if (noOfProducts > 0) {
                try (BufferedWriter writer = new BufferedWriter(new FileWriter(ordersOut, true))) {
                    writer.write(orderId + "," + noOfProducts + ",shipped" + "\n");
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private static class SecondaryWorker implements Runnable {
        private String orderId;
        private int noOfProducts;
        private String productsFilePath;
        private String ordersOut;
        private String orderProductsOut;
        private AtomicInteger currentProduct;
        private PrimaryWorker primaryWorker;

        private SecondaryWorker(String orderId, int noOfProducts, String productsFilePath, String ordersOut,
                String orderProductsOut, AtomicInteger currentProduct, PrimaryWorker primaryWorker) {
            this.orderId = orderId;
            this.noOfProducts = noOfProducts;
            this.productsFilePath = productsFilePath;
            this.ordersOut = ordersOut;
            this.orderProductsOut = orderProductsOut;
            this.currentProduct = currentProduct;
            this.primaryWorker = primaryWorker;
        }

        @Override
        public void run() {
            BufferedReader reader = null;
            try {
                reader = new BufferedReader(new FileReader(productsFilePath));
                String line;
                int countProduct = 0;
                while ((line = reader.readLine()) != null) {
                    String[] parts = line.split(",");
                    String orderId2 = parts[0];
                    if (orderId.equals(orderId2)) {
                        if (countProduct == currentProduct.get()) {
                            try (BufferedWriter writer = new BufferedWriter(new FileWriter(orderProductsOut, true))) {
                                writer.write(line.split("\n")[0] + ",shipped" + "\n");
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                            synchronized (primaryWorker) {
                                primaryWorker.remainingWorkers--;
                                primaryWorker.notify();
                            }
                            break;
                        }
                        countProduct++;
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                try {
                    if (reader != null) {
                        reader.close();
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
