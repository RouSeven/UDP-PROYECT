const ws = new WebSocket(`ws://${window.location.host}`);
const sendBtn = document.getElementById("sendBtn");
const messageInput = document.getElementById("messageInput");
const chatLog = document.getElementById("chatLog");
const usernameInput = document.getElementById("username");

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
