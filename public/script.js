const ws = new WebSocket(`ws://${window.location.host}`);
const sendBtn = document.getElementById("sendBtn");
const messageInput = document.getElementById("messageInput");
const chatLog = document.getElementById("chatLog");
const usernameInput = document.getElementById("username");
const fileInput = document.getElementById("fileInput");
const uploadBtn = document.getElementById("uploadBtn");

ws.onmessage = (event) => {
  const p = document.createElement("p");
  p.classList.add("mb-1");
  p.textContent = event.data;
  chatLog.appendChild(p);
  chatLog.scrollTop = chatLog.scrollHeight;
};

sendBtn.addEventListener("click", () => {
  const username = usernameInput.value.trim();
  const message = messageInput.value.trim();

  if (!username || !message) return;

  const fullMessage = `[${username}] ${message}`;
  ws.send(fullMessage);
  messageInput.value = "";
});

messageInput.addEventListener("keyup", (e) => {
  if (e.key === "Enter") sendBtn.click();
});

uploadBtn.addEventListener("click", () => {
  const username = usernameInput.value.trim();
  const file = fileInput.files[0];

  if (!username || !file) return;

  const reader = new FileReader();

  reader.onload = () => {
    const fileData = reader.result.split(',')[1]; // base64
    const payload = {
      type: "file",
      username,
      filename: file.name,
      content: fileData
    };

    ws.send(JSON.stringify(payload));

    const p = document.createElement("p");
    p.classList.add("mb-1");
    p.textContent = `[${username}] envió archivo: ${file.name}`;
    chatLog.appendChild(p);
    chatLog.scrollTop = chatLog.scrollHeight;
  };

  reader.readAsDataURL(file);
});
