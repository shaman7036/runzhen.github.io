---
title: "root @ Matrix #"  
pagetitle: 空谷幽兰
layout: page
---

<ul class="listing">
{% for post in site.posts %}
  {% capture y %}{{post.date | date:"%Y"}}{% endcapture %}
  {% if year != y %}
    {% assign year = y %}
    <li class="listing-seperator">{{ y }}</li>
  {% endif %}
  <li class="listing-item">
    <time datetime="{{ post.date | date:"%Y-%m-%d" }}">{{ post.date | date:"%Y-%m-%d" }}</time>
    <a href="{{ site.url }}{{ post.url }}" title="{{ post.title }}">{{ post.title }}</a>
  </li>
{% endfor %}
</ul>

<div id="post-pagination" class="paginator">
  {% if site.previous_page %}
    {% if site.previous_page == 1 %}
    <a href="/">&lt;tPrevious</a>
    {% else %}
    <a href="/page{{site.previous_page}}">&lt;Previous</a>
    {% endif %}
  {% else %}
    <span class="previous disabled">&lt;Previous</span>
  {% endif %}

      {% if site.page == 1 %}
      <span class="current-page">1</span>
      {% else %}
      <a href="/">1</a>
      {% endif %}

    {% for count in (2..site.total_pages) %}
      {% if count == site.page %}
      <span class="current-page">{{count}}</span>
      {% else %}
      <a href="/page{{count}}">{{count}}</a>
      {% endif %}
    {% endfor %}

  {% if site.next_page %}
    <a class="next" href="/page{{site.next_page}}">Next&gt;</a>
  {% else %}
    <span class="next disabled" >Next&gt;</span>
  {% endif %}
  ({{ site.total_posts }} posts in total)
</div>


