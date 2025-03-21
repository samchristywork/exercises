use std::io::Result;
use std::io::prelude::*;
use std::net::{TcpListener, TcpStream};
use std::thread;
use std::time::Duration;
use std::sync::{Arc, Mutex, mpsc};

struct ThreadPool {
    workers: Vec<Worker>,
    sender: mpsc::Sender<Message>,
}

enum Message {
    NewJob(Job),
    Terminate,
}

trait FnBox {
    fn call_box(self: Box<Self>);
}

impl<F: FnOnce()> FnBox for F {
    fn call_box(self: Box<Self>) {
        (*self)()
    }
}

type Job = Box<dyn FnBox + Send + 'static>;

impl ThreadPool {
    fn new(size: usize) -> ThreadPool {
        assert!(size > 0);

        let (sender, receiver) = mpsc::channel();
        let receiver = Arc::new(Mutex::new(receiver));

        let mut workers = Vec::with_capacity(size);

        for id in 0..size {
            let worker = Worker::new(id, Arc::clone(&receiver));
            println!("Worker {} was created", worker.id);
            workers.push(worker);
        }

        ThreadPool { workers, sender }
    }

    fn execute<F>(&self, f: F)
    where
        F: FnOnce() + Send + 'static,
    {
        let job = Box::new(f);
        self.sender.send(Message::NewJob(job)).unwrap();
    }
}

impl Drop for ThreadPool {
    fn drop(&mut self) {
        for _ in &self.workers {
            self.sender.send(Message::Terminate).unwrap();
        }

        for worker in &mut self.workers {
            if let Some(thread) = worker.thread.take() {
                thread.join().unwrap();
            }
        }
    }
}

struct Worker {
    id: usize,
    thread: Option<thread::JoinHandle<()>>,
}

impl Worker {
    fn new(id: usize, receiver: Arc<Mutex<mpsc::Receiver<Message>>>) -> Worker {
        let thread = thread::spawn(move || loop {
            let message = receiver.lock().unwrap().recv().unwrap();

            match message {
                Message::NewJob(job) => {
                    println!("Worker {} got a job; executing.", id);
                    job.call_box();
                }
                Message::Terminate => {
                    println!("Worker {} was told to terminate.", id);
                    break;
                }
            }
        });

        Worker {
            id,
            thread: Some(thread),
        }
    }
}

fn handle_hello(_request: &str) -> (String, String, String) {
    ("Hello, World!".to_string(), "200 OK".to_string(), "text/plain".to_string())
}

fn handle_goodbye(_request: &str) -> (String, String, String) {
    ("Bye!".to_string(), "200 OK".to_string(), "text/plain".to_string())
}

fn handle_echo_request(request: &str) -> (String, String, String) {
    let lines: Vec<&str> = request.lines().collect();
    let mut response = "<!doctype html><html><body><h1>Request</h1><pre>".to_string();
    for line in lines {
        response.push_str("<div>");
        response.push_str(line);
        response.push_str("</div>");
    }
    response.push_str("</pre></body></html>");

    (response, "200 OK".to_string(), "text/html".to_string())
}

fn handle_index(_request: &str) -> (String, String, String) {
    let bytes = include_bytes!("../index.html");
    let content = String::from_utf8_lossy(bytes);
    (content.to_string(), "200 OK".to_string(), "text/html".to_string())
}

fn handle_connection(mut stream: TcpStream, timeout: u64) -> Result<()> {
    stream.set_read_timeout(Some(Duration::new(timeout, 0)))?;

    let mut buffer = [0; 8192];
    match stream.read(&mut buffer) {
        Ok(_) => {
            println!("Request: {}", String::from_utf8_lossy(&buffer[..]));

            let (response, status, content_type) = match String::from_utf8_lossy(&buffer[..]).as_ref() {
                s if s.starts_with("GET /hello") => handle_hello(s),
                s if s.starts_with("GET /goodbye") => handle_goodbye(s),
                s if s.starts_with("GET /echo-request") => handle_echo_request(s),
                s if s.starts_with("GET /") => handle_index(s),
                _ => ("Unknown request".to_string(), "404 Not Found".to_string(), "text/plain".to_string()),
            };

            let content_length = response.len();
            let response = format!(
                "HTTP/1.1 {}\r\nContent-Length: {}\r\nContent-Type: {}\r\n\r\n{}",
                 status, content_length, content_type, response
            );
            stream.write_all(response.as_bytes())?;
            stream.flush()?;
        }
        Err(e) => {
            eprintln!("Failed to read from stream: {}", e);
        }
    }

    Ok(())
}

fn main() {
}
