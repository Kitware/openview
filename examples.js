
function popover(d) {
  $(this).popover();
}

function areaClicked() {
  var area = d3.select(this);
  var name = area.text().toLowerCase();
  var prevClass = area.attr("class");
  d3.select("#areas").selectAll("h4").attr("class", "");
  var showAll = prevClass === "text-success";
  if (!showAll) {
    area.attr("class", "text-success");
  }
  d3.select("#examples").selectAll("li")
    .each(function(d) {
      if (showAll || d.tags.indexOf(name) >= 0) {
        $(this).show("fast");
      } else {
        $(this).hide("fast");
      }
    });
}

d3.json("examples.json", function(data) {

  d3.select("#examples").selectAll("li").data(data)
    .enter().append("li")
      .attr("class", "span2")
      .attr("title", function(d) { return d.name; })
      .attr("data-placement", "top")
      .attr("data-content", function(d) { return d.description; })
      .attr("data-trigger", "hover")
      .each(popover)
    .append("a")
      .attr("href", function(d) { return d.url; })
      .attr("class", "thumbnail")
    .append("img")
      .attr("src", function(d) { return d.thumbnail; })
      .attr("alt", function(d) { return d.name; });

  d3.select("#areas").selectAll("h4")
    .on("click", areaClicked);

});

