#include "OcTree.hpp"

void OcTree::AddPoint(const glm::vec3 &point)
{
	if (!m_Oct.Contains(point))
		return;

	if (m_Points.size() < m_Capacity)
	{
		m_Points.push_back(point);
		return;
	}

	if (!m_SubDivided)
	{
		m_Points.push_back(point);
		SubDivide();
		m_SubDivided = true;
	}
	
	for (const auto &octs : m_Children)
	{
		octs->AddPoint(point);
	}
}

void OcTree::Generate(const uint32_t &size, const std::vector<glm::vec3> points)
{
	m_Children.clear();
	m_Points.clear();
	m_SubDivided = false;
	m_Size = size;
	
	glm::vec3 origin = glm::vec3((float)(size) / 2.0f, (float)(size) / 2.0f, (float)(size) / 2.0f);
	glm::vec3 dim = glm::vec3(size, size, size);

	m_Oct = AABB(origin, dim);


	for (const auto &point : points)
		AddPoint(point);
}

int OcTree::GetPointCount()
{
	int count = (m_Capacity == 0) ? 0 : m_Points.size();
	for (int i = 0; i < m_Children.size(); i++)
	{
		count += m_Children[i]->GetPointCount();
	}
	return count;
}

int OcTree::GetOctCount()
{
	int count = 1;
	for (int i = 0; i < m_Children.size(); i++)
	{
		count += m_Children[i]->GetOctCount();
	}
	return count;
}

void OcTree::GetAllPoints(std::vector<glm::vec3> &points)
{
	for (const auto &point : m_Points)
	{
		if (m_Size == 1) points.push_back(point);
	}
}

void OcTree::SubDivide()
{
	uint32_t size = m_Size / 2;
	float sizef = (float)m_Size / 4.0f;
	
	glm::vec3 dims = glm::vec3(sizef, sizef, sizef);

	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x - sizef, m_Oct.Origin.y - sizef, m_Oct.Origin.z - sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x + sizef, m_Oct.Origin.y - sizef, m_Oct.Origin.z - sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x - sizef, m_Oct.Origin.y - sizef, m_Oct.Origin.z + sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x + sizef, m_Oct.Origin.y - sizef, m_Oct.Origin.z + sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x - sizef, m_Oct.Origin.y + sizef, m_Oct.Origin.z - sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x + sizef, m_Oct.Origin.y + sizef, m_Oct.Origin.z - sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x - sizef, m_Oct.Origin.y + sizef, m_Oct.Origin.z + sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x + sizef, m_Oct.Origin.y + sizef, m_Oct.Origin.z + sizef), dims), size));
}
