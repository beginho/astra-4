function update(){
  fetch('/api/stats').then(r=>r.json()).then(d=>{
    document.getElementById('uptime').textContent = d.uptime;
  });
}
function stopServer(){
  fetch('/api/control?cmd=stop').then(r=>r.json()).then(d=>{
    alert('Server stopping');
  });
}
setInterval(update,1000);
update();

