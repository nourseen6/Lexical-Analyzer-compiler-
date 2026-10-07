const slides = Array.from(document.querySelectorAll(".slide"));
const prevBtn = document.getElementById("prevBtn");
const nextBtn = document.getElementById("nextBtn");
const counter = document.getElementById("slideCounter");
const progressBar = document.getElementById("progressBar");

let currentSlide = 0;

function showSlide(index) {
  currentSlide = Math.max(0, Math.min(index, slides.length - 1));

  slides.forEach((slide, slideIndex) => {
    slide.classList.toggle("active", slideIndex === currentSlide);
  });

  prevBtn.disabled = currentSlide === 0;
  nextBtn.disabled = currentSlide === slides.length - 1;
  counter.textContent = `${currentSlide + 1} / ${slides.length}`;
  progressBar.style.width = `${((currentSlide + 1) / slides.length) * 100}%`;
  document.title = `${slides[currentSlide].dataset.title} - Lexical Analyzer Presentation`;
}

prevBtn.addEventListener("click", () => showSlide(currentSlide - 1));
nextBtn.addEventListener("click", () => showSlide(currentSlide + 1));

document.addEventListener("keydown", (event) => {
  if (event.key === "ArrowRight" || event.key === "PageDown" || event.key === " ") {
    event.preventDefault();
    showSlide(currentSlide + 1);
  }

  if (event.key === "ArrowLeft" || event.key === "PageUp") {
    event.preventDefault();
    showSlide(currentSlide - 1);
  }

  if (event.key === "Home") {
    showSlide(0);
  }

  if (event.key === "End") {
    showSlide(slides.length - 1);
  }
});

showSlide(0);
